// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "docrrecognition.h"
#include "docrrecognition_p.h"
#include "aidaemon_sessionmanager.h"
#include "daierror.h"

#include <QDBusConnection>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMutexLocker>

DCORE_USE_NAMESPACE
DAI_BEGIN_NAMESPACE

static constexpr int REQ_TIMEOUT = 30000;

DOCRRecognitionPrivate::DOCRRecognitionPrivate(DOCRRecognition *parent)
    : QObject(parent)
    , q(parent)
{
}

DOCRRecognitionPrivate::~DOCRRecognitionPrivate()
{
    if (ocrIfs && !sessionId.isEmpty()) {
        ocrIfs->terminate();
    }
}

bool DOCRRecognitionPrivate::ensureServer()
{
    if (ocrIfs.isNull() || !ocrIfs->isValid()) {
        QDBusConnection con = QDBusConnection::sessionBus();
        OrgDeepinAiDaemonSessionManagerInterface sessionManager(OrgDeepinAiDaemonSessionManagerInterface::staticInterfaceName(),
                                                                "/org/deepin/ai/daemon/SessionManager", con);
        if (!sessionManager.isValid())
            return false;

        sessionId = sessionManager.CreateSession("OCR");
        if (sessionId.isEmpty())
            return false;

        QString sessionPath = QString("/org/deepin/ai/daemon/Session/%1").arg(sessionId);
        ocrIfs.reset(new OrgDeepinAiDaemonSessionOCRInterface(OrgDeepinAiDaemonSessionManagerInterface::staticInterfaceName(), sessionPath, con));
        ocrIfs->setTimeout(REQ_TIMEOUT);
    }

    return ocrIfs->isValid();
}

QString DOCRRecognitionPrivate::packageParams(const QVariantHash &params)
{
    QVariantHash root;
    
    // Merge user-provided parameters
    for (auto it = params.begin(); it != params.end(); ++it) {
        root.insert(it.key(), it.value());
    }

    QString ret = QString::fromUtf8(QJsonDocument::fromVariant(root).toJson(QJsonDocument::Compact));
    return ret;
}

// Note: Removed async signal handlers since using synchronous interface

DOCRRecognition::DOCRRecognition(QObject *parent)
    : QObject(parent)
    , d(new DOCRRecognitionPrivate(this))
{
}

DOCRRecognition::~DOCRRecognition()
{
}

QString DOCRRecognition::recognizeFile(const QString &imageFile, const QVariantHash &params)
{
    if (!d->ensureServer()) {
        d->error = DError(AIErrorCode::APIServerNotAvailable, "");
        return QString();
    }
    
    if (imageFile.isEmpty()) {
        d->error = DError(AIErrorCode::InvalidParameter, "Empty image file path");
        return QString();
    }
    
    QMutexLocker lk(&d->mtx);
    d->running = true;
    d->error = DError();
    
    QString ret = d->ocrIfs->recognizeFile(imageFile, d->packageParams(params));
    
    // Parse result
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(ret.toUtf8(), &error);
    if (error.error != QJsonParseError::NoError) {
        d->error = DError(AIErrorCode::ParseError, error.errorString());
        d->running = false;
        return QString();
    }
    
    QJsonObject obj = doc.object();
    if (obj.contains("error") && obj["error"].toBool()) {
        int errorCode = obj["error_code"].toInt();
        QString errorMessage = obj["error_message"].toString();
        d->error = DError(errorCode, errorMessage);
        d->running = false;
        return QString();
    }
    
    d->running = false;
    return obj["text"].toString();
}

QString DOCRRecognition::recognizeImage(const QByteArray &imageData, const QVariantHash &params)
{
    if (!d->ensureServer()) {
        d->error = DError(AIErrorCode::APIServerNotAvailable, "");
        return QString();
    }
    
    if (imageData.isEmpty()) {
        d->error = DError(AIErrorCode::InvalidParameter, "Empty image data");
        return QString();
    }
    
    QMutexLocker lk(&d->mtx);
    d->running = true;
    d->error = DError();
    
    QString ret = d->ocrIfs->recognizeImage(imageData, d->packageParams(params));
    
    // Parse result
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(ret.toUtf8(), &error);
    if (error.error != QJsonParseError::NoError) {
        d->error = DError(AIErrorCode::ParseError, error.errorString());
        d->running = false;
        return QString();
    }
    
    QJsonObject obj = doc.object();
    if (obj.contains("error") && obj["error"].toBool()) {
        int errorCode = obj["error_code"].toInt();
        QString errorMessage = obj["error_message"].toString();
        d->error = DError(errorCode, errorMessage);
        d->running = false;
        return QString();
    }
    
    d->running = false;
    return obj["text"].toString();
}

QString DOCRRecognition::recognizeRegionFromString(const QString &imageFile, const QString &region, const QVariantHash &params)
{
    if (!d->ensureServer()) {
        d->error = DError(AIErrorCode::APIServerNotAvailable, "");
        return QString();
    }
    
    if (imageFile.isEmpty()) {
        d->error = DError(AIErrorCode::InvalidParameter, "Empty image file path");
        return QString();
    }
    
    if (region.isEmpty()) {
        d->error = DError(AIErrorCode::InvalidParameter, "Empty region");
        return QString();
    }
    
    QMutexLocker lk(&d->mtx);
    d->running = true;
    d->error = DError();
    
    QString ret = d->ocrIfs->recognizeRegion(imageFile, region, d->packageParams(params));
    
    // Parse result
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(ret.toUtf8(), &error);
    if (error.error != QJsonParseError::NoError) {
        d->error = DError(AIErrorCode::ParseError, error.errorString());
        d->running = false;
        return QString();
    }
    
    QJsonObject obj = doc.object();
    if (obj.contains("error") && obj["error"].toBool()) {
        int errorCode = obj["error_code"].toInt();
        QString errorMessage = obj["error_message"].toString();
        d->error = DError(errorCode, errorMessage);
        d->running = false;
        return QString();
    }
    
    d->running = false;
    return obj["text"].toString();
}

QString DOCRRecognition::recognizeRegionFromRect(const QString &imageFile, const QRect &region, const QVariantHash &params)
{
    // Convert QRect to string format: "x,y,width,height"
    QString regionStr = QString("%1,%2,%3,%4")
                        .arg(region.x())
                        .arg(region.y())
                        .arg(region.width())
                        .arg(region.height());
    
    return recognizeRegionFromString(imageFile, regionStr, params);
}

QStringList DOCRRecognition::getSupportedLanguages()
{
    if (!d->ensureServer()) {
        d->error = DError(AIErrorCode::APIServerNotAvailable, "");
        return QStringList();
    }
    
    return d->ocrIfs->getSupportedLanguages();
}

QStringList DOCRRecognition::getSupportedFormats()
{
    if (!d->ensureServer()) {
        d->error = DError(AIErrorCode::APIServerNotAvailable, "");
        return QStringList();
    }
    
    return d->ocrIfs->getSupportedFormats();
}

QString DOCRRecognition::getCapabilities()
{
    if (!d->ensureServer()) {
        d->error = DError(AIErrorCode::APIServerNotAvailable, "");
        return QString();
    }
    
    return d->ocrIfs->getCapabilities();
}

// Note: cancel method removed - not applicable for synchronous interface

void DOCRRecognition::terminate()
{
    if (d->ocrIfs)
        d->ocrIfs->terminate();
        
    QMutexLocker lk(&d->mtx);
    d->running = false;
}

DError DOCRRecognition::lastError() const
{
    return d->error;
}

DAI_END_NAMESPACE

#include "docrrecognition.moc"
