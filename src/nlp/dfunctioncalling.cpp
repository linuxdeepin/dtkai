// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dfunctioncalling_p.h"
#include "aidaemon_sessionmanager.h"
#include "daierror.h"

#include <QMutexLocker>
#include <QJsonDocument>

DCORE_USE_NAMESPACE
DAI_USE_NAMESPACE

#define CHAT_TIMEOUT 30 * 1000  
#define REQ_TIMEOUT  10 * 1000

DFunctionCallingPrivate::DFunctionCallingPrivate(DFunctionCalling *parent) : q(parent)
{

}

DFunctionCallingPrivate::~DFunctionCallingPrivate()
{
    if (!funcIfs.isNull() && !sessionId.isEmpty()) {
        OrgDeepinAiDaemonSessionManagerInterface sessionManager(OrgDeepinAiDaemonSessionManagerInterface::staticInterfaceName(),
                                                                "/org/deepin/ai/daemon/SessionManager", QDBusConnection::sessionBus());
        if (sessionManager.isValid())
            sessionManager.DestroySession(sessionId);

        funcIfs.reset(nullptr);
    }
}

bool DFunctionCallingPrivate::ensureServer()
{
    if (funcIfs.isNull() || !funcIfs->isValid()) {
        QDBusConnection con = QDBusConnection::sessionBus();
        OrgDeepinAiDaemonSessionManagerInterface sessionManager(OrgDeepinAiDaemonSessionManagerInterface::staticInterfaceName(),
                                                                "/org/deepin/ai/daemon/SessionManager", con);
        if (!sessionManager.isValid())
            return false;

        sessionId = sessionManager.CreateSession("FunctionCalling");
        if (sessionId.isEmpty())
            return false;

        QString sessionPath = QString("/org/deepin/ai/daemon/Session/%1").arg(sessionId);
        funcIfs.reset(new OrgDeepinAiDaemonSessionFunctionCallingInterface(OrgDeepinAiDaemonSessionManagerInterface::staticInterfaceName(), sessionPath, con));
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

