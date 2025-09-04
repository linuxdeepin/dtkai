// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dchatcompletions_p.h"
#include "aidaemon_sessionmanager.h"
#include "daierror.h"

#include <QMutexLocker>
#include <QJsonDocument>

DCORE_USE_NAMESPACE
DAI_USE_NAMESPACE

#define CHAT_TIMEOUT 30 * 1000  
#define REQ_TIMEOUT  10 * 1000

DChatCompletionsPrivate::DChatCompletionsPrivate(DChatCompletions *parent)
    : QObject()
    , q(parent)
{

}

DChatCompletionsPrivate::~DChatCompletionsPrivate()
{
    if (!chatIfs.isNull() && !sessionId.isEmpty()) {
        OrgDeepinAiDaemonSessionManagerInterface sessionManager(OrgDeepinAiDaemonSessionManagerInterface::staticInterfaceName(),
                                                                "/org/deepin/ai/daemon/SessionManager", QDBusConnection::sessionBus());
        if (sessionManager.isValid())
            sessionManager.DestroySession(sessionId);

        chatIfs.reset(nullptr);
    }
}

bool DChatCompletionsPrivate::ensureServer()
{
    if (chatIfs.isNull() || !chatIfs->isValid()) {
        QDBusConnection con = QDBusConnection::sessionBus();
        OrgDeepinAiDaemonSessionManagerInterface sessionManager(OrgDeepinAiDaemonSessionManagerInterface::staticInterfaceName(),
                                                                "/org/deepin/ai/daemon/SessionManager", con);
        if (!sessionManager.isValid())
            return false;

        sessionId = sessionManager.CreateSession("Chat");
        if (sessionId.isEmpty())
            return false;

        QString sessionPath = QString("/org/deepin/ai/daemon/Session/%1").arg(sessionId);
        chatIfs.reset(new OrgDeepinAiDaemonSessionChatInterface(OrgDeepinAiDaemonSessionManagerInterface::staticInterfaceName(), sessionPath, con));
        chatIfs->setTimeout(REQ_TIMEOUT);
        q->connect(chatIfs.data(), &OrgDeepinAiDaemonSessionChatInterface::StreamOutput, q, &DChatCompletions::streamOutput);
        q->connect(chatIfs.data(), &OrgDeepinAiDaemonSessionChatInterface::StreamFinished, this, &DChatCompletionsPrivate::finished);
    }

    return chatIfs->isValid();
}

QString DChatCompletionsPrivate::packageParams(const QList<ChatHistory> &history, const QVariantHash &params)
{
    QVariantHash root;

    // Add chat history to messages
    QVariantList msgs;
    for (const ChatHistory &chat : history) {
        QVariantHash obj;
        obj.insert("role", chat.role);
        obj.insert("content", chat.content);
        msgs.append(obj);
    }
    root.insert("messages", msgs);
    
    // Merge user-provided parameters (including model, temperature, etc.)
    for (auto it = params.begin(); it != params.end(); ++it) {
        root.insert(it.key(), it.value());
    }

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

    d->chatIfs->streamChat(prompt, d->packageParams(history, params));
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
    QString ret = d->chatIfs->chat(prompt, d->packageParams(history, params));
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
        d->chatIfs->terminate();
}

DError DChatCompletions::lastError() const
{
    return d->error;
}


