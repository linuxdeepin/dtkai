// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DSPEECHTOTEXT_P_H
#define DSPEECHTOTEXT_P_H

#include "dspeechtotext.h"
#include "aidaemon_apisession_speechtotext.h"

DAI_BEGIN_NAMESPACE

class DSpeechToTextPrivate : public QObject
{
    Q_OBJECT
public:
    explicit DSpeechToTextPrivate(DSpeechToText *q);
    ~DSpeechToTextPrivate();
    bool ensureServer();
    static QString packageParams(const QVariantHash &params);
    
public Q_SLOTS:
    void onRecognitionResult(const QString &streamSessionId, const QString &text);
    void onRecognitionPartialResult(const QString &streamSessionId, const QString &partialText);
    void onRecognitionError(const QString &streamSessionId, int errorCode, const QString &errorMessage);
    void onRecognitionCompleted(const QString &streamSessionId, const QString &finalText);
    
public:
    QMutex mtx;
    bool running = false;
    DTK_CORE_NAMESPACE::DError error;
    QScopedPointer<OrgDeepinAiDaemonSessionSpeechToTextInterface> speechIfs;
    QString sessionId;
    QString currentStreamSessionId;
    
public:
    DSpeechToText *q = nullptr;
};

DAI_END_NAMESPACE

#endif // DSPEECHTOTEXT_P_H 