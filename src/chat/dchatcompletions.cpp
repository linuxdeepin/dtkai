// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dchatcompletions_p.h"
#include "aidaemon_apiserver.h"
#include "dtkaierror.h"

#include <QMutexLocker>
#include <QJsonDocument>

DCORE_USE_NAMESPACE
DAI_USE_NAMESPACE

#define CHAT_TIMEOUT 5 * 60 * 1000
#define REQ_TIMEOUT  10 * 1000

DChatCompletionsPrivate::DChatCompletionsPrivate(DChatCompletions *parent)
    : QObject()
    , q(parent)
{

}

DChatCompletionsPrivate::~DChatCompletionsPrivate()
{
    if (!chatIfs.isNull()) {
        OrgDeepinAiDaemonAPIServerInterface server(OrgDeepinAiDaemonAPIServerInterface::staticInterfaceName(),
                                                   "/org/deepin/ai/daemon/APIServer", QDBusConnection::sessionBus());
        if (server.isValid())
            server.DestorySession(chatIfs->path());

        chatIfs.reset(nullptr);
    }
}

bool DChatCompletionsPrivate::ensureServer()
{
    if (chatIfs.isNull() || !chatIfs->isValid()) {
        QDBusConnection con = QDBusConnection::sessionBus();
        OrgDeepinAiDaemonAPIServerInterface server(OrgDeepinAiDaemonAPIServerInterface::staticInterfaceName(),
                                                   "/org/deepin/ai/daemon/APIServer", con);
        if (!server.isValid())
            return false;

        QString path = server.CreateSession("chat");
        if (path.isEmpty())
            return false;

        chatIfs.reset(new OrgDeepinAiDaemonAPISessionChatInterface(OrgDeepinAiDaemonAPIServerInterface::staticInterfaceName(), path, con));
        chatIfs->setTimeout(REQ_TIMEOUT);
        q->connect(chatIfs.data(), &OrgDeepinAiDaemonAPISessionChatInterface::StreamOutput, q, &DChatCompletions::streamOutput);
        q->connect(chatIfs.data(), &OrgDeepinAiDaemonAPISessionChatInterface::StreamFinished, this, &DChatCompletionsPrivate::finished);
    }

    return chatIfs->isValid();
}

QString DChatCompletionsPrivate::packageParams(const QList<ChatHistory> &history, const QVariantHash &params)
{
    QVariantHash root;

    QVariantList msgs;
    for (const ChatHistory &chat : history) {
        QVariantHash obj;
        obj.insert("role", chat.role);
        obj.insert("content", chat.content);
        msgs.append(obj);
    }

    root.insert("messages", msgs);
    QString ret = QString::fromUtf8(QJsonDocument::fromVariant(root).toJson(QJsonDocument::Compact));
    return ret;
}

void DChatCompletionsPrivate::finished(int err, const QString &content)
{
    QMutexLocker lk(&mtx);
    running = false;
    error.setErrorCode(err);
    error.setErrorMessage(err == 0 ? QString() : content);
    lk.unlock();

    emit q->streamFinished(err);
}

DChatCompletions::DChatCompletions(QObject *parent)
    : QObject(parent)
    , d(new DChatCompletionsPrivate(this))
{

}

DChatCompletions::~DChatCompletions()
{

}

bool DChatCompletions::chatStream(const QString &prompt, const QList<ChatHistory> &history, const QVariantHash &params)
{
    QMutexLocker lk(&d->mtx);
    if (d->running)
        return false;

    if (!d->ensureServer()) {
        d->error = DError(AIErrorCode::APIServerNotAvailable, "");
        return false;
    }

    d->running = true;
    lk.unlock();

    d->chatIfs->StreamChat(prompt, d->packageParams(history, params));
    return true;
}

QString DChatCompletions::chat(const QString &prompt, const QList<ChatHistory> &history, const QVariantHash &params)
{
    QMutexLocker lk(&d->mtx);
    if (d->running)
        return "";

    if (!d->ensureServer()) {
        d->error = DError(AIErrorCode::APIServerNotAvailable, "");
        return "";
    }

    d->running = true;
    lk.unlock();

    d->chatIfs->setTimeout(CHAT_TIMEOUT);
    QString ret = d->chatIfs->Chat(prompt, d->packageParams(history, params));
    d->chatIfs->setTimeout(REQ_TIMEOUT);
    {
        QJsonDocument doc = QJsonDocument::fromJson(ret.toUtf8());
        auto var = doc.object().toVariantHash();
        if (var.contains("error")) {
            d->error = DError(var.value("error").toInt(), var.value("errorMessage").toString());
        } else {
            ret = var.value("content").toString();
            d->error = DError(0, "");
        }
    }

    lk.relock();
    d->running = false;
    return ret;
}

void DChatCompletions::terminate()
{
    if (d->chatIfs)
        d->chatIfs->Terminate();
}

DError DChatCompletions::lastError() const
{
    return d->error;
}


