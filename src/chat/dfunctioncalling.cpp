// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dfunctioncalling_p.h"
#include "aidaemon_apiserver.h"
#include "dtkaierror.h"

#include <QMutexLocker>
#include <QJsonDocument>

DCORE_USE_NAMESPACE
DAI_USE_NAMESPACE

#define CHAT_TIMEOUT 5 * 60 * 1000
#define REQ_TIMEOUT  10 * 1000

DFunctionCallingPrivate::DFunctionCallingPrivate(DFunctionCalling *parent) : q(parent)
{

}

DFunctionCallingPrivate::~DFunctionCallingPrivate()
{
    if (!funcIfs.isNull()) {
        OrgDeepinAiDaemonAPIServerInterface server(OrgDeepinAiDaemonAPIServerInterface::staticInterfaceName(),
                                                   "/org/deepin/ai/daemon/APIServer", QDBusConnection::sessionBus());
        if (server.isValid())
            server.DestorySession(funcIfs->path());

        funcIfs.reset(nullptr);
    }
}

bool DFunctionCallingPrivate::ensureServer()
{
    if (funcIfs.isNull() || !funcIfs->isValid()) {
        QDBusConnection con = QDBusConnection::sessionBus();
        OrgDeepinAiDaemonAPIServerInterface server(OrgDeepinAiDaemonAPIServerInterface::staticInterfaceName(),
                                                   "/org/deepin/ai/daemon/APIServer", con);
        if (!server.isValid())
            return false;

        QString path = server.CreateSession("function_calling");
        if (path.isEmpty())
            return false;

        funcIfs.reset(new OrgDeepinAiDaemonAPISessionFunctionCallingInterface(OrgDeepinAiDaemonAPIServerInterface::staticInterfaceName(), path, con));
        funcIfs->setTimeout(REQ_TIMEOUT);
    }

    return funcIfs->isValid();
}

QString DFunctionCallingPrivate::packageParams(const QVariantHash &params)
{
    if (params.isEmpty())
        return "";

    return QString::fromUtf8(QJsonDocument(QJsonObject::fromVariantHash(params))
                             .toJson(QJsonDocument::Compact));
}

DFunctionCalling::DFunctionCalling(QObject *parent)
    : QObject(parent)
     , d(new DFunctionCallingPrivate(this))
{

}

DFunctionCalling::~DFunctionCalling()
{

}

QString DFunctionCalling::parse(const QString &prompt, const QString &functions, const QVariantHash &params)
{
    if (prompt.isEmpty() || functions.isEmpty())
        return "";

    QMutexLocker lk(&d->mtx);
    if (d->running)
        return "";

    if (!d->ensureServer()) {
        d->error = DError(AIErrorCode::APIServerNotAvailable, "");
        return "";
    }

    d->running = true;
    lk.unlock();

    d->funcIfs->setTimeout(CHAT_TIMEOUT);
    QString ret = d->funcIfs->Parse(prompt, functions, d->packageParams(params));
    d->funcIfs->setTimeout(REQ_TIMEOUT);
    {
        QJsonDocument doc = QJsonDocument::fromJson(ret.toUtf8());
        auto var = doc.object().toVariantHash();
        if (var.contains("error")) {
            d->error = DError(var.value("error").toInt(), var.value("errorMessage").toString());
        } else {
            auto jObj = QJsonObject::fromVariantHash(var.value("function").value<QVariantHash>());
            QJsonDocument doc(jObj);
            ret = QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
            d->error = DError(0, "");
        }
    }

    lk.relock();
    d->running = false;
    return ret;
}

void DFunctionCalling::terminate()
{
    if (d->funcIfs)
        d->funcIfs->Terminate();
}

DError DFunctionCalling::lastError() const
{
    return d->error;
}

