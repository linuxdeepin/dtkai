// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DIMAGERECOGNITION_H
#define DIMAGERECOGNITION_H


#include "dtkai_global.h"
#include <DError>

#include <QObject>
#include <QVariantHash>

DAI_BEGIN_NAMESPACE


/**
 * @brief Client interface for image recognition services
 * 
 * This class provides a high-level interface for recognizing and analyzing images
 * using AI services. It supports multiple input methods and provides both
 * synchronous operations.
 */
class DImageRecognitionPrivate;
class DImageRecognition : public QObject
{
    Q_OBJECT
    
public:
    explicit DImageRecognition(QObject *parent = nullptr);
    ~DImageRecognition();
    
    // Synchronous recognition methods
    QString recognizeImage(const QString &imagePath, const QString &prompt = QString(), 
                          const QVariantHash &params = {});
    QString recognizeImageData(const QByteArray &imageData, const QString &prompt = QString(),
                              const QVariantHash &params = {});
    QString recognizeImageUrl(const QString &imageUrl, const QString &prompt = QString(),
                             const QVariantHash &params = {});
    
    // Information query methods
    QStringList getSupportedImageFormats();
    int getMaxImageSize();
    
    // Error handling
    DTK_CORE_NAMESPACE::DError lastError() const;
    
    // Control methods
    void terminate();
    
private:
    QScopedPointer<DImageRecognitionPrivate> d;
};

DAI_END_NAMESPACE

#endif // DIMAGERECOGNITION_H

