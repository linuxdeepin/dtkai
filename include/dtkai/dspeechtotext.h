// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DSPEECHTOTEXT_H
#define DSPEECHTOTEXT_H

#include "dtkai_global.h"
#include "dtkaitypes.h"

#include <DError>

#include <QObject>
#include <QScopedPointer>
#include <QVariant>
#include <QStringList>

DAI_BEGIN_NAMESPACE
class DSpeechToTextPrivate;
class DSpeechToText : public QObject
{
    Q_OBJECT
    friend class DSpeechToTextPrivate;
public:
    explicit DSpeechToText(QObject *parent = nullptr);
    ~DSpeechToText();
    
    // File-based recognition
    QString recognizeFile(const QString &audioFile, const QVariantHash &params = {});
    
    // Stream-based recognition
    bool startStreamRecognition(const QVariantHash &params = {});
    bool sendAudioData(const QByteArray &audioData);
    QString endStreamRecognition();
    
    // Control methods
    void terminate();
    
    // Information methods
    QStringList getSupportedFormats();
    
    // Error handling
    DTK_CORE_NAMESPACE::DError lastError() const;
    
Q_SIGNALS:
    // Stream recognition signals
    void recognitionResult(const QString &text);
    void recognitionPartialResult(const QString &partialText);
    void recognitionError(int errorCode, const QString &errorMessage);
    void recognitionCompleted(const QString &finalText);
    
private:
    QScopedPointer<DSpeechToTextPrivate> d;
};

DAI_END_NAMESPACE

#endif // DSPEECHTOTEXT_H 
