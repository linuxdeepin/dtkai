// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dimagerecognition.h"
#include "dimagerecognition_p.h"
#include "aidaemon_sessionmanager.h"
#include "daierror.h"

#include <QDBusConnection>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMutexLocker>

DCORE_USE_NAMESPACE
DAI_USE_NAMESPACE

static constexpr int REQ_TIMEOUT = 30000;

DImageRecognitionPrivate::DImageRecognitionPrivate(DImageRecognition *parent)
    : QObject(parent)
    , q(parent)
{
}

DImageRecognitionPrivate::~DImageRecognitionPrivate()
{
    if (imageIfs && !sessionId.isEmpty()) {
        imageIfs->terminate();
    }
}

bool DImageRecognitionPrivate::ensureServer()
{
    if (imageIfs.isNull() || !imageIfs->isValid()) {
        QDBusConnection con = QDBusConnection::sessionBus();
        OrgDeepinAiDaemonSessionManagerInterface sessionManager(OrgDeepinAiDaemonSessionManagerInterface::staticInterfaceName(),
                                                                "/org/deepin/ai/daemon/SessionManager", con);
        if (!sessionManager.isValid())
            return false;

        sessionId = sessionManager.CreateSession("ImageRecognition");
        if (sessionId.isEmpty())
            return false;

        QString sessionPath = QString("/org/deepin/ai/daemon/Session/%1").arg(sessionId);
        imageIfs.reset(new OrgDeepinAiDaemonSessionImageRecognitionInterface(OrgDeepinAiDaemonSessionManagerInterface::staticInterfaceName(), sessionPath, con));
        imageIfs->setTimeout(REQ_TIMEOUT);
    }

    return imageIfs->isValid();
}

QString DImageRecognitionPrivate::packageParams(const QVariantHash &params)
{
    QVariantHash root;
    
    // Merge user-provided parameters
    for (auto it = params.begin(); it != params.end(); ++it) {
        root.insert(it.key(), it.value());
    }

    QString ret = QString::fromUtf8(QJsonDocument::fromVariant(root).toJson(QJsonDocument::Compact));
    return ret;
}



DImageRecognition::DImageRecognition(QObject *parent)
    : QObject(parent)
    , d(new DImageRecognitionPrivate(this))
{
}

DImageRecognition::~DImageRecognition()
{
}

QString DImageRecognition::recognizeImage(const QString &imagePath, const QString &prompt, const QVariantHash &params)
{
    if (!d->ensureServer()) {
        d->error = DError(AIErrorCode::APIServerNotAvailable, "");
        return QString();
    }
    
    if (imagePath.isEmpty()) {
        d->error = DError(AIErrorCode::InvalidParameter, "Empty image path");
        return QString();
    }
    
    // Add path security check
    QFileInfo fileInfo(imagePath);
    if (!fileInfo.isAbsolute()) {
        d->error = DError(AIErrorCode::InvalidParameter, "Relative path not allowed for security reasons");
        return QString();
    }
    
    if (!fileInfo.exists()) {
        d->error = DError(AIErrorCode::InvalidParameter, "Image file does not exist");
        return QString();
    }
    
    QMutexLocker lk(&d->mtx);
    d->running = true;
    d->error = DError();
    
    QString ret = d->imageIfs->recognizeImage(imagePath, prompt, d->packageParams(params));
    
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
    return obj["content"].toString();
}

QString DImageRecognition::recognizeImageData(const QByteArray &imageData, const QString &prompt, const QVariantHash &params)
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
    
    QString ret = d->imageIfs->recognizeImageData(imageData, prompt, d->packageParams(params));
    
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
    return obj["content"].toString();
}

QString DImageRecognition::recognizeImageUrl(const QString &imageUrl, const QString &prompt, const QVariantHash &params)
{
    if (!d->ensureServer()) {
        d->error = DError(AIErrorCode::APIServerNotAvailable, "");
        return QString();
    }
    
    if (imageUrl.isEmpty()) {
        d->error = DError(AIErrorCode::InvalidParameter, "Empty image URL");
        return QString();
    }
    
    QMutexLocker lk(&d->mtx);
    d->running = true;
    d->error = DError();
    
    QString ret = d->imageIfs->recognizeImageUrl(imageUrl, prompt, d->packageParams(params));
    
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
    return obj["content"].toString();
}

QStringList DImageRecognition::getSupportedImageFormats()
{
    if (!d->ensureServer()) {
        d->error = DError(AIErrorCode::APIServerNotAvailable, "");
        return QStringList();
    }
    
    return d->imageIfs->getSupportedImageFormats();
}

int DImageRecognition::getMaxImageSize()
{
    if (!d->ensureServer()) {
        d->error = DError(AIErrorCode::APIServerNotAvailable, "");
        return 0;
    }
    
    return d->imageIfs->getMaxImageSize();
}

void DImageRecognition::terminate()
{
    if (d->imageIfs)
        d->imageIfs->terminate();
        
    QMutexLocker lk(&d->mtx);
    d->running = false;
}

DError DImageRecognition::lastError() const
{
    return d->error;
}

#include "dimagerecognition.moc"

