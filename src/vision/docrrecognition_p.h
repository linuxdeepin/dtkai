// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DOCRRECOGNITION_P_H
#define DOCRRECOGNITION_P_H

#include "docrrecognition.h"
#include "aidaemon_apisession_ocr.h"

#include <QObject>
#include <QMutex>
#include <QScopedPointer>

DCORE_USE_NAMESPACE
DAI_BEGIN_NAMESPACE

class DOCRRecognitionPrivate : public QObject
{
    Q_OBJECT
    
public:
    explicit DOCRRecognitionPrivate(DOCRRecognition *parent);
    ~DOCRRecognitionPrivate();
    
    bool ensureServer();
    QString packageParams(const QVariantHash &params);
    
// Note: No async signals needed for synchronous interface
    
public:
    DOCRRecognition *q = nullptr;
    QString sessionId;
    QScopedPointer<OrgDeepinAiDaemonSessionOCRInterface> ocrIfs;
    
    mutable QMutex mtx;
    bool running = false;
    DTK_CORE_NAMESPACE::DError error;
};

DAI_END_NAMESPACE

#endif // DOCRRECOGNITION_P_H
