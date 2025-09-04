// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DTEXTTOSPEECH_P_H
#define DTEXTTOSPEECH_P_H

#include "dtexttospeech.h"
#include "aidaemon_apisession_texttospeech.h"

DAI_BEGIN_NAMESPACE

class DTextToSpeechPrivate : public QObject
{
    Q_OBJECT
public:
    explicit DTextToSpeechPrivate(DTextToSpeech *q);
    ~DTextToSpeechPrivate();
    bool ensureServer();
    static QString packageParams(const QVariantHash &params);
    
public Q_SLOTS:
    void onSynthesisResult(const QString &streamSessionId, const QByteArray &audioData);
    void onSynthesisError(const QString &streamSessionId, int errorCode, const QString &errorMessage);
    void onSynthesisCompleted(const QString &streamSessionId, const QByteArray &finalAudio);
    
public:
    QMutex mtx;
    bool running = false;
    DTK_CORE_NAMESPACE::DError error;
    QScopedPointer<OrgDeepinAiDaemonSessionTextToSpeechInterface> ttsIfs;
    QString sessionId;
    QString currentStreamSessionId;
    
public:
    DTextToSpeech *q = nullptr;
};

DAI_END_NAMESPACE

#endif // DTEXTTOSPEECH_P_H
