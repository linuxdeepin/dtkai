// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dtexttospeech_p.h"
#include "aidaemon_sessionmanager.h"
#include "daierror.h"

#include <QMutexLocker>
#include <QJsonDocument>

DCORE_USE_NAMESPACE
DAI_USE_NAMESPACE

#define SYNTHESIS_TIMEOUT 30 * 1000  // 30 seconds
#define REQ_TIMEOUT  10 * 1000

DTextToSpeechPrivate::DTextToSpeechPrivate(DTextToSpeech *parent)
    : QObject()
    , q(parent)
{

}

DTextToSpeechPrivate::~DTextToSpeechPrivate()
{
    if (!ttsIfs.isNull() && !sessionId.isEmpty()) {
        OrgDeepinAiDaemonSessionManagerInterface sessionManager(OrgDeepinAiDaemonSessionManagerInterface::staticInterfaceName(),
                                                                "/org/deepin/ai/daemon/SessionManager", QDBusConnection::sessionBus());
        if (sessionManager.isValid())
            sessionManager.DestroySession(sessionId);

        ttsIfs.reset(nullptr);
    }
}

bool DTextToSpeechPrivate::ensureServer()
{
    if (ttsIfs.isNull() || !ttsIfs->isValid()) {
        QDBusConnection con = QDBusConnection::sessionBus();
        OrgDeepinAiDaemonSessionManagerInterface sessionManager(OrgDeepinAiDaemonSessionManagerInterface::staticInterfaceName(),
                                                                "/org/deepin/ai/daemon/SessionManager", con);
        if (!sessionManager.isValid())
            return false;

        sessionId = sessionManager.CreateSession("TextToSpeech");
        if (sessionId.isEmpty())
            return false;

        QString sessionPath = QString("/org/deepin/ai/daemon/Session/%1").arg(sessionId);
        ttsIfs.reset(new OrgDeepinAiDaemonSessionTextToSpeechInterface(OrgDeepinAiDaemonSessionManagerInterface::staticInterfaceName(), sessionPath, con));
        ttsIfs->setTimeout(REQ_TIMEOUT);
        
        // Connect signals
        q->connect(ttsIfs.data(), &OrgDeepinAiDaemonSessionTextToSpeechInterface::SynthesisResult, 
                   this, &DTextToSpeechPrivate::onSynthesisResult);
        q->connect(ttsIfs.data(), &OrgDeepinAiDaemonSessionTextToSpeechInterface::SynthesisError, 
                   this, &DTextToSpeechPrivate::onSynthesisError);
        q->connect(ttsIfs.data(), &OrgDeepinAiDaemonSessionTextToSpeechInterface::SynthesisCompleted, 
                   this, &DTextToSpeechPrivate::onSynthesisCompleted);
    }

    return ttsIfs->isValid();
}

QString DTextToSpeechPrivate::packageParams(const QVariantHash &params)
{
    QVariantHash root;
    
    // Merge user-provided parameters
    for (auto it = params.begin(); it != params.end(); ++it) {
        root.insert(it.key(), it.value());
    }

    QString ret = QString::fromUtf8(QJsonDocument::fromVariant(root).toJson(QJsonDocument::Compact));
    return ret;
}

void DTextToSpeechPrivate::onSynthesisResult(const QString &streamSessionId, const QByteArray &audioData)
{
    if (streamSessionId == currentStreamSessionId) {
        emit q->synthesisResult(audioData);
    }
}

void DTextToSpeechPrivate::onSynthesisError(const QString &streamSessionId, int errorCode, const QString &errorMessage)
{
    if (streamSessionId == currentStreamSessionId) {
        QMutexLocker lk(&mtx);
        running = false;
        error.setErrorCode(errorCode);
        error.setErrorMessage(errorMessage);
        lk.unlock();
        
        emit q->synthesisError(errorCode, errorMessage);
    }
}

void DTextToSpeechPrivate::onSynthesisCompleted(const QString &streamSessionId, const QByteArray &finalAudio)
{
    if (streamSessionId == currentStreamSessionId) {
        QMutexLocker lk(&mtx);
        running = false;
        error.setErrorCode(0);
        error.setErrorMessage("");
        lk.unlock();
        
        emit q->synthesisCompleted(finalAudio);
    }
}

DTextToSpeech::DTextToSpeech(QObject *parent)
    : QObject(parent)
    , d(new DTextToSpeechPrivate(this))
{

}

DTextToSpeech::~DTextToSpeech()
{

}

QByteArray DTextToSpeech::synthesizeText(const QString &text, const QVariantHash &params)
{
    QMutexLocker lk(&d->mtx);
    if (d->running)
        return QByteArray();

    if (!d->ensureServer()) {
        d->error = DError(AIErrorCode::APIServerNotAvailable, "");
        return QByteArray();
    }

    d->running = true;
    lk.unlock();

    d->ttsIfs->setTimeout(SYNTHESIS_TIMEOUT);
    QString taskId = d->ttsIfs->synthesizeText(text, d->packageParams(params));
    d->ttsIfs->setTimeout(REQ_TIMEOUT);
    
    // For now, we'll return empty data since synthesis is asynchronous
    // In a real implementation, we would wait for the synthesis result
    QByteArray result;
    
    {
        QJsonDocument doc = QJsonDocument::fromJson(taskId.toUtf8());
        auto var = doc.object().toVariantHash();
        int errorCode = var.value("error_code", 0).toInt();
        if (errorCode != 0) {
            d->error = DError(errorCode, var.value("error_message").toString());
            result = QByteArray();
        } else {
            d->error = DError(0, "");
        }
    }

    lk.relock();
    d->running = false;
    return result;
}

bool DTextToSpeech::startStreamSynthesis(const QString &text, const QVariantHash &params)
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

    QString streamSessionId = d->ttsIfs->startStreamSynthesis(text, d->packageParams(params));
    if (streamSessionId.isEmpty()) {
        lk.relock();
        d->running = false;
        d->error = DError(AIErrorCode::APIServerNotAvailable, "Failed to start stream synthesis");
        return false;
    }
    
    d->currentStreamSessionId = streamSessionId;
    return true;
}

QByteArray DTextToSpeech::endStreamSynthesis()
{
    if (!d->ttsIfs || d->currentStreamSessionId.isEmpty())
        return QByteArray();
        
    QString result = d->ttsIfs->endStreamSynthesis(d->currentStreamSessionId);
    d->currentStreamSessionId.clear();
    
    QMutexLocker lk(&d->mtx);
    d->running = false;
    
    // Parse result to get audio data
    QByteArray audioData;
    QJsonDocument doc = QJsonDocument::fromJson(result.toUtf8());
    auto var = doc.object().toVariantHash();
    int errorCode = var.value("error_code", 0).toInt();
    if (errorCode == 0) {
        audioData = QByteArray::fromBase64(var.value("audio_data").toString().toUtf8());
    } else {
        d->error = DError(errorCode, var.value("error_message").toString());
    }
    
    return audioData;
}

void DTextToSpeech::terminate()
{
    if (d->ttsIfs)
        d->ttsIfs->terminate();
        
    QMutexLocker lk(&d->mtx);
    d->running = false;
    d->currentStreamSessionId.clear();
}

QStringList DTextToSpeech::getSupportedVoices()
{
    if (!d->ensureServer()) {
        d->error = DError(AIErrorCode::APIServerNotAvailable, "");
        return QStringList();
    }
    
    return d->ttsIfs->getSupportedVoices();
}

DError DTextToSpeech::lastError() const
{
    return d->error;
}
