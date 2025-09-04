// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dspeechtotext_p.h"
#include "aidaemon_sessionmanager.h"
#include "daierror.h"

#include <QMutexLocker>
#include <QJsonDocument>

DCORE_USE_NAMESPACE
DAI_USE_NAMESPACE

#define RECOGNITION_TIMEOUT 30 * 1000  // 30 seconds 
#define REQ_TIMEOUT  10 * 1000

DSpeechToTextPrivate::DSpeechToTextPrivate(DSpeechToText *parent)
    : QObject()
    , q(parent)
{

}

DSpeechToTextPrivate::~DSpeechToTextPrivate()
{
    if (!speechIfs.isNull() && !sessionId.isEmpty()) {
        OrgDeepinAiDaemonSessionManagerInterface sessionManager(OrgDeepinAiDaemonSessionManagerInterface::staticInterfaceName(),
                                                                "/org/deepin/ai/daemon/SessionManager", QDBusConnection::sessionBus());
        if (sessionManager.isValid())
            sessionManager.DestroySession(sessionId);

        speechIfs.reset(nullptr);
    }
}

bool DSpeechToTextPrivate::ensureServer()
{
    if (speechIfs.isNull() || !speechIfs->isValid()) {
        QDBusConnection con = QDBusConnection::sessionBus();
        OrgDeepinAiDaemonSessionManagerInterface sessionManager(OrgDeepinAiDaemonSessionManagerInterface::staticInterfaceName(),
                                                                "/org/deepin/ai/daemon/SessionManager", con);
        if (!sessionManager.isValid())
            return false;

        sessionId = sessionManager.CreateSession("SpeechToText");
        if (sessionId.isEmpty())
            return false;

        QString sessionPath = QString("/org/deepin/ai/daemon/Session/%1").arg(sessionId);
        speechIfs.reset(new OrgDeepinAiDaemonSessionSpeechToTextInterface(OrgDeepinAiDaemonSessionManagerInterface::staticInterfaceName(), sessionPath, con));
        speechIfs->setTimeout(REQ_TIMEOUT);
        
        // Connect signals
        q->connect(speechIfs.data(), &OrgDeepinAiDaemonSessionSpeechToTextInterface::RecognitionResult, 
                   this, &DSpeechToTextPrivate::onRecognitionResult);
        q->connect(speechIfs.data(), &OrgDeepinAiDaemonSessionSpeechToTextInterface::RecognitionPartialResult, 
                   this, &DSpeechToTextPrivate::onRecognitionPartialResult);
        q->connect(speechIfs.data(), &OrgDeepinAiDaemonSessionSpeechToTextInterface::RecognitionError, 
                   this, &DSpeechToTextPrivate::onRecognitionError);
        q->connect(speechIfs.data(), &OrgDeepinAiDaemonSessionSpeechToTextInterface::RecognitionCompleted, 
                   this, &DSpeechToTextPrivate::onRecognitionCompleted);
    }

    return speechIfs->isValid();
}

QString DSpeechToTextPrivate::packageParams(const QVariantHash &params)
{
    QVariantHash root;
    
    // Merge user-provided parameters
    for (auto it = params.begin(); it != params.end(); ++it) {
        root.insert(it.key(), it.value());
    }

    QString ret = QString::fromUtf8(QJsonDocument::fromVariant(root).toJson(QJsonDocument::Compact));
    return ret;
}

void DSpeechToTextPrivate::onRecognitionResult(const QString &streamSessionId, const QString &text)
{
    if (streamSessionId == currentStreamSessionId) {
        emit q->recognitionResult(text);
    }
}

void DSpeechToTextPrivate::onRecognitionPartialResult(const QString &streamSessionId, const QString &partialText)
{
    if (streamSessionId == currentStreamSessionId) {
        emit q->recognitionPartialResult(partialText);
    }
}

void DSpeechToTextPrivate::onRecognitionError(const QString &streamSessionId, int errorCode, const QString &errorMessage)
{
    if (streamSessionId == currentStreamSessionId) {
        QMutexLocker lk(&mtx);
        running = false;
        error.setErrorCode(errorCode);
        error.setErrorMessage(errorMessage);
        lk.unlock();
        
        emit q->recognitionError(errorCode, errorMessage);
    }
}

void DSpeechToTextPrivate::onRecognitionCompleted(const QString &streamSessionId, const QString &finalText)
{
    if (streamSessionId == currentStreamSessionId) {
        QMutexLocker lk(&mtx);
        running = false;
        error.setErrorCode(0);
        error.setErrorMessage("");
        lk.unlock();
        
        emit q->recognitionCompleted(finalText);
    }
}

DSpeechToText::DSpeechToText(QObject *parent)
    : QObject(parent)
    , d(new DSpeechToTextPrivate(this))
{

}

DSpeechToText::~DSpeechToText()
{

}

QString DSpeechToText::recognizeFile(const QString &audioFile, const QVariantHash &params)
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

    d->speechIfs->setTimeout(RECOGNITION_TIMEOUT);
    QString ret = d->speechIfs->recognizeFile(audioFile, d->packageParams(params));
    d->speechIfs->setTimeout(REQ_TIMEOUT);
    
    {
        QJsonDocument doc = QJsonDocument::fromJson(ret.toUtf8());
        auto var = doc.object().toVariantHash();
        int errorCode = var.value("error_code", 0).toInt();
        if (errorCode != 0) {
            d->error = DError(errorCode, var.value("error_message").toString());
            ret = "";
        } else {
            ret = var.value("text").toString();
            d->error = DError(0, "");
        }
    }

    lk.relock();
    d->running = false;
    return ret;
}

bool DSpeechToText::startStreamRecognition(const QVariantHash &params)
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

    QString streamSessionId = d->speechIfs->startStreamRecognition(d->packageParams(params));
    if (streamSessionId.isEmpty()) {
        lk.relock();
        d->running = false;
        d->error = DError(AIErrorCode::APIServerNotAvailable, "Failed to start stream recognition");
        return false;
    }
    
    d->currentStreamSessionId = streamSessionId;
    return true;
}

bool DSpeechToText::sendAudioData(const QByteArray &audioData)
{
    if (!d->speechIfs || d->currentStreamSessionId.isEmpty())
        return false;
        
    return d->speechIfs->sendAudioData(d->currentStreamSessionId, audioData);
}

QString DSpeechToText::endStreamRecognition()
{
    if (!d->speechIfs || d->currentStreamSessionId.isEmpty())
        return "";
        
    QString result = d->speechIfs->endStreamRecognition(d->currentStreamSessionId);
    d->currentStreamSessionId.clear();
    
    QMutexLocker lk(&d->mtx);
    d->running = false;
    
    return result;
}

void DSpeechToText::terminate()
{
    if (d->speechIfs)
        d->speechIfs->terminate();
        
    QMutexLocker lk(&d->mtx);
    d->running = false;
    d->currentStreamSessionId.clear();
}

QStringList DSpeechToText::getSupportedFormats()
{
    if (!d->ensureServer()) {
        d->error = DError(AIErrorCode::APIServerNotAvailable, "");
        return QStringList();
    }
    
    return d->speechIfs->getSupportedFormats();
}

DError DSpeechToText::lastError() const
{
    return d->error;
} 
