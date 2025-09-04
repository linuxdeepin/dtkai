// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DOCRRECOGNITION_H
#define DOCRRECOGNITION_H

#include "dtkai_global.h"
#include <DError>
#include <QObject>
#include <QVariantHash>
#include <QRect>

DAI_BEGIN_NAMESPACE

class DOCRRecognitionPrivate;
class DOCRRecognition : public QObject
{
    Q_OBJECT
    
public:
    explicit DOCRRecognition(QObject *parent = nullptr);
    ~DOCRRecognition();
    
    // Synchronous OCR methods
    QString recognizeFile(const QString &imageFile, const QVariantHash &params = {});
    QString recognizeImage(const QByteArray &imageData, const QVariantHash &params = {});
    
    /**
     * @brief Recognize text within a specific region of an image
     * @param imageFile Path to the image file to be analyzed. Must be an absolute path for security reasons.
     * @param region String definition of the region to analyze (e.g., "10,20,100,50" for x,y,width,height)
     * @param params Optional parameters to customize the OCR behavior (language, format options, etc.)
     * @return Recognized text as a QString, or empty string if an error occurred
     */
    QString recognizeRegionFromString(const QString &imageFile, const QString &region, const QVariantHash &params = {});
    
    /**
     * @brief Recognize text within a specific rectangular region of an image
     * @param imageFile Path to the image file to be analyzed. Must be an absolute path for security reasons.
     * @param region QRect object defining the region to analyze (x, y, width, height)
     * @param params Optional parameters to customize the OCR behavior (language, format options, etc.)
     * @return Recognized text as a QString, or empty string if an error occurred
     */
    QString recognizeRegionFromRect(const QString &imageFile, const QRect &region, const QVariantHash &params = {});
    
    // Information query methods
    QStringList getSupportedLanguages();
    QStringList getSupportedFormats();
    QString getCapabilities();
    
    // Control methods  
    void terminate();
    
    // Error handling
    DTK_CORE_NAMESPACE::DError lastError() const;
    
private:
    QScopedPointer<DOCRRecognitionPrivate> d;
};

DAI_END_NAMESPACE

#endif // DOCRRECOGNITION_H
