// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DTEXTTOSPEECH_H
#define DTEXTTOSPEECH_H

#include "dtkai_global.h"
#include "dtkaitypes.h"

#include <DError>

#include <QObject>
#include <QScopedPointer>
#include <QVariant>
#include <QStringList>

DAI_BEGIN_NAMESPACE
class DTextToSpeechPrivate;
class DTextToSpeech : public QObject
{
    Q_OBJECT
    friend class DTextToSpeechPrivate;
public:
    explicit DTextToSpeech(QObject *parent = nullptr);
    ~DTextToSpeech();
    
    // Text synthesis
    QByteArray synthesizeText(const QString &text, const QVariantHash &params = {});
    
    // Stream-based synthesis
    bool startStreamSynthesis(const QString &text, const QVariantHash &params = {});
    QByteArray endStreamSynthesis();
    
    // Control methods
    void terminate();
    
    // Information methods
    QStringList getSupportedVoices();
    
    // Error handling
    DTK_CORE_NAMESPACE::DError lastError() const;
    
Q_SIGNALS:
    // Stream synthesis signals
    void synthesisResult(const QByteArray &audioData);
    void synthesisError(int errorCode, const QString &errorMessage);
    void synthesisCompleted(const QByteArray &finalAudio);
    
private:
    QScopedPointer<DTextToSpeechPrivate> d;
};

DAI_END_NAMESPACE

#endif // DTEXTTOSPEECH_H
