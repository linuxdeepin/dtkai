// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "nlp/dembeddingplatform.h"
#include "dembeddingplatform_p.h"
#include "daierror.h"

#include <DError>

#include <QDBusInterface>
#include <QDBusPendingReply>
#include <QDBusConnection>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

DAI_BEGIN_NAMESPACE

DEmbeddingPlatformPrivate::DEmbeddingPlatformPrivate(DEmbeddingPlatform *qq)
    : DObjectPrivate(qq)
{
    error = DTK_CORE_NAMESPACE::DError(NoError, "");
}

DEmbeddingPlatform::DEmbeddingPlatform(QObject *parent)
    : QObject(parent)
    , DObject(*new DEmbeddingPlatformPrivate(this))
{
}

DEmbeddingPlatform::~DEmbeddingPlatform() = default;

QString DEmbeddingPlatform::embeddingModels()
{
    D_D(DEmbeddingPlatform);
    QDBusInterface interface("org.deepin.ai.daemon",
                             "/org/deepin/ai/daemon/EmbeddingPlatform",
                             "org.deepin.ai.daemon.EmbeddingPlatform",
                             QDBusConnection::sessionBus());
    QDBusPendingReply<QString> reply = interface.asyncCall("embeddingModels");
    reply.waitForFinished();
    if (reply.isError()) {
        qWarning() << "DBus error:" << reply.error().message();
        d->error = DTK_CORE_NAMESPACE::DError(AIErrorCode::APIServerNotAvailable, reply.error().message());
        return QString();
    }
    d->error = DTK_CORE_NAMESPACE::DError(NoError, "");
    return reply.value();
}

QList<DEmbeddingPlatform::DocumentInfo> DEmbeddingPlatform::uploadDocuments(const QString &appId, const QStringList &files, const QString &extensionParams)
{
    D_D(DEmbeddingPlatform);
    QDBusInterface interface("org.deepin.ai.daemon",
                             "/org/deepin/ai/daemon/EmbeddingPlatform",
                             "org.deepin.ai.daemon.EmbeddingPlatform",
                             QDBusConnection::sessionBus());
    QDBusPendingReply<QString> reply = interface.asyncCall("uploadDocuments", appId, files, extensionParams);
    reply.waitForFinished();
    if (reply.isError()) {
        qWarning() << "DBus error:" << reply.error().message();
        d->error = DTK_CORE_NAMESPACE::DError(AIErrorCode::APIServerNotAvailable, reply.error().message());
        return QList<DocumentInfo>();
    }
    
    // Parse JSON response
    QString response = reply.value();
    QJsonDocument doc = QJsonDocument::fromJson(response.toUtf8());
    if (!doc.isObject()) {
        qWarning() << "Invalid JSON response:" << response;
        d->error = DTK_CORE_NAMESPACE::DError(AIErrorCode::APIServerNotAvailable, "Invalid JSON response");
        return QList<DocumentInfo>();
    }
    
    QJsonObject obj = doc.object();
    QList<DocumentInfo> infos;
    
    // Check if results field exists and is an array
    if (!obj.contains("results") || !obj["results"].isArray()) {
        qWarning() << "Missing or invalid results field in response:" << response;
        d->error = DTK_CORE_NAMESPACE::DError(AIErrorCode::APIServerNotAvailable, "Missing or invalid results field in response");
        return QList<DocumentInfo>();
    }
    
    QJsonArray resultsArray = obj["results"].toArray();
    
    // Iterate through each element in the results array
    for (const QJsonValue value : resultsArray) {
        if (!value.isObject()) {
            continue;
        }
        
        QJsonObject itemObj = value.toObject();
        DocumentInfo info;
        
        // Extract fields according to the provided JSON format
        if (itemObj.contains("documentID") && itemObj["documentID"].isString()) {
            info.id = itemObj["documentID"].toString();
        }
        
        if (itemObj.contains("file") && itemObj["file"].isString()) {
            info.filePath = itemObj["file"].toString();
        }
        
        // Note: The JSON format doesn't include createdAt or metadata fields
        // We'll leave them as default values
        
        infos.append(info);
    }
    
    d->error = DTK_CORE_NAMESPACE::DError(NoError, "");
    return infos;
}

bool DEmbeddingPlatform::deleteDocuments(const QString &appId, const QStringList &documentIds)
{
    D_D(DEmbeddingPlatform);
    QDBusInterface interface("org.deepin.ai.daemon",
                             "/org/deepin/ai/daemon/EmbeddingPlatform",
                             "org.deepin.ai.daemon.EmbeddingPlatform",
                             QDBusConnection::sessionBus());
    QDBusPendingReply<QString> reply = interface.asyncCall("deleteDocuments", appId, documentIds);
    reply.waitForFinished();
    if (reply.isError()) {
        qWarning() << "DBus error:" << reply.error().message();
        d->error = DTK_CORE_NAMESPACE::DError(AIErrorCode::APIServerNotAvailable, reply.error().message());
        return false;
    }

    QString response = reply.value();
    QJsonDocument doc = QJsonDocument::fromJson(response.toUtf8());
    if (!doc.isObject()) {
        qWarning() << "Invalid JSON response:" << response;
        d->error = DTK_CORE_NAMESPACE::DError(AIErrorCode::APIServerNotAvailable, "Invalid JSON response");
        return false;
    }

    d->error = DTK_CORE_NAMESPACE::DError(NoError, "");
    return true;
}

QList<DEmbeddingPlatform::SearchResult> DEmbeddingPlatform::search(const QString &appId, const QString &query, const QString &extensionParams)
{
    D_D(DEmbeddingPlatform);
    QDBusInterface interface("org.deepin.ai.daemon",
                             "/org/deepin/ai/daemon/EmbeddingPlatform",
                             "org.deepin.ai.daemon.EmbeddingPlatform",
                             QDBusConnection::sessionBus());
    QDBusPendingReply<QString> reply = interface.asyncCall("search", appId, query, extensionParams);
    reply.waitForFinished();
    if (reply.isError()) {
        qWarning() << "DBus error:" << reply.error().message();
        d->error = DTK_CORE_NAMESPACE::DError(AIErrorCode::APIServerNotAvailable, reply.error().message());
        return QList<SearchResult>();
    }
    
    // Parse JSON response
    QString response = reply.value();
    QJsonDocument doc = QJsonDocument::fromJson(response.toUtf8());
    if (!doc.isObject()) {
        qWarning() << "Invalid JSON response:" << response;
        d->error = DTK_CORE_NAMESPACE::DError(AIErrorCode::APIServerNotAvailable, "Invalid JSON response");
        return QList<SearchResult>();
    }
    
    QJsonObject obj = doc.object();
    QList<SearchResult> results;
    
    // Check if results field exists and is an array
    if (!obj.contains("results") || !obj["results"].isArray()) {
        qWarning() << "Missing or invalid results field in response:" << response;
        d->error = DTK_CORE_NAMESPACE::DError(AIErrorCode::APIServerNotAvailable, "Missing or invalid results field in response");
        return QList<SearchResult>();
    }
    
    QJsonArray resultsArray = obj["results"].toArray();
    
    // Iterate through each element in the results array
    for (const QJsonValue value : resultsArray) {
        if (!value.isObject()) {
            continue;
        }
        
        QJsonObject itemObj = value.toObject();
        SearchResult result;
        
        // Extract fields
        if (itemObj.contains("id") && itemObj["id"].isString()) {
            result.id = itemObj["id"].toString();
        }
        
        if (itemObj.contains("model") && itemObj["model"].isString()) {
            result.model = itemObj["model"].toString();
        }
        
        if (itemObj.contains("distance") && itemObj["distance"].isDouble()) {
            result.distance = itemObj["distance"].toDouble();
        }
        
        // Extract chunk object
        if (itemObj.contains("chunk") && itemObj["chunk"].isObject()) {
            QJsonObject chunkObj = itemObj["chunk"].toObject();
            
            if (chunkObj.contains("chunk_index") && chunkObj["chunk_index"].isDouble()) {
                result.chunk.chunkIndex = chunkObj["chunk_index"].toInt();
            }
            
            if (chunkObj.contains("content") && chunkObj["content"].isString()) {
                result.chunk.content = chunkObj["content"].toString();
            }
            
            if (chunkObj.contains("tokens") && chunkObj["tokens"].isDouble()) {
                result.chunk.tokens = chunkObj["tokens"].toInt();
            }
            
            if (chunkObj.contains("timestamp") && chunkObj["timestamp"].isArray()) {
                QJsonArray timestampArray = chunkObj["timestamp"].toArray();
                for (const QJsonValue &tsValue : timestampArray) {
                    if (tsValue.isString()) {
                        result.chunk.timestamp.append(tsValue.toString());
                    }
                }
            }
        }
        
        results.append(result);
    }
    
    d->error = DTK_CORE_NAMESPACE::DError(NoError, "");
    return results;
}

bool DEmbeddingPlatform::cancelTask(const QString &taskId)
{
    D_D(DEmbeddingPlatform);
    QDBusInterface interface("org.deepin.ai.daemon",
                             "/org/deepin/ai/daemon/EmbeddingPlatform",
                             "org.deepin.ai.daemon.EmbeddingPlatform",
                             QDBusConnection::sessionBus());
    QDBusPendingReply<bool> reply = interface.asyncCall("cancelTask", taskId);
    reply.waitForFinished();
    if (reply.isError()) {
        qWarning() << "DBus error:" << reply.error().message();
        d->error = DTK_CORE_NAMESPACE::DError(AIErrorCode::APIServerNotAvailable, reply.error().message());
        return false;
    }
    d->error = DTK_CORE_NAMESPACE::DError(NoError, "");
    return reply.value();
}

QList<DEmbeddingPlatform::DocumentInfo> DEmbeddingPlatform::documentsInfo(const QString &appId, const QStringList &documentIds)
{
    D_D(DEmbeddingPlatform);
    QDBusInterface interface("org.deepin.ai.daemon",
                             "/org/deepin/ai/daemon/EmbeddingPlatform",
                             "org.deepin.ai.daemon.EmbeddingPlatform",
                             QDBusConnection::sessionBus());
    QDBusPendingReply<QString> reply = interface.asyncCall("documentsInfo", appId, documentIds);
    reply.waitForFinished();
    if (reply.isError()) {
        qWarning() << "DBus error:" << reply.error().message();
        d->error = DTK_CORE_NAMESPACE::DError(AIErrorCode::APIServerNotAvailable, reply.error().message());
        return QList<DocumentInfo>();
    }
    
    // Parse JSON response
    QString response = reply.value();
    QJsonDocument doc = QJsonDocument::fromJson(response.toUtf8());
    if (!doc.isObject()) {
        qWarning() << "Invalid JSON response:" << response;
        d->error = DTK_CORE_NAMESPACE::DError(AIErrorCode::APIServerNotAvailable, "Invalid JSON response");
        return QList<DocumentInfo>();
    }
    
    QJsonObject obj = doc.object();
    QList<DocumentInfo> infos;
    
    // Check if results field exists and is an array
    if (!obj.contains("results") || !obj["results"].isArray()) {
        qWarning() << "Missing or invalid results field in response:" << response;
        d->error = DTK_CORE_NAMESPACE::DError(AIErrorCode::APIServerNotAvailable, "Missing or invalid results field in response");
        return QList<DocumentInfo>();
    }
    
    QJsonArray resultsArray = obj["results"].toArray();
    
    // Iterate through each element in the results array
    for (const QJsonValue value : resultsArray) {
        if (!value.isObject()) {
            continue;
        }
        
        QJsonObject itemObj = value.toObject();
        DocumentInfo info;
        
        // Extract fields
        if (itemObj.contains("id") && itemObj["id"].isString()) {
            info.id = itemObj["id"].toString();
        }
        
        if (itemObj.contains("file_path") && itemObj["file_path"].isString()) {
            info.filePath = itemObj["file_path"].toString();
        }
        
        if (itemObj.contains("created_at") && itemObj["created_at"].isString()) {
            QString dateStr = itemObj["created_at"].toString();
            info.createdAt = QDateTime::fromString(dateStr, Qt::ISODate);
        } else {
            info.createdAt = std::nullopt;
        }
        
        // Extract metadata
        if (itemObj.contains("metadata") && itemObj["metadata"].isObject()) {
            info.metadata = itemObj["metadata"].toObject().toVariantMap();
        }
        
        infos.append(info);
    }
    
    d->error = DTK_CORE_NAMESPACE::DError(NoError, "");
    return infos;
}

bool DEmbeddingPlatform::buildIndex(const QString &appId, const QString &docId, const QString &extensionParams)
{
    D_D(DEmbeddingPlatform);
    QDBusInterface interface("org.deepin.ai.daemon",
                             "/org/deepin/ai/daemon/EmbeddingPlatform",
                             "org.deepin.ai.daemon.EmbeddingPlatform",
                             QDBusConnection::sessionBus());
    QDBusPendingReply<QString> reply = interface.asyncCall("buildIndex", appId, docId, extensionParams);
    reply.waitForFinished();
    if (reply.isError()) {
        qWarning() << "DBus error:" << reply.error().message();
        d->error = DTK_CORE_NAMESPACE::DError(AIErrorCode::APIServerNotAvailable, reply.error().message());
        return false;
    }

    // Parse JSON response to extract success field
    QString response = reply.value();
    QJsonDocument doc = QJsonDocument::fromJson(response.toUtf8());
    if (!doc.isObject()) {
        qWarning() << "Invalid JSON response:" << response;
        d->error = DTK_CORE_NAMESPACE::DError(AIErrorCode::APIServerNotAvailable, "Invalid JSON response");
        return false;
    }

    return true;
}

bool DEmbeddingPlatform::destroyIndex(const QString &appId, bool allIndex, const QString &extensionParams)
{
    D_D(DEmbeddingPlatform);
    QDBusInterface interface("org.deepin.ai.daemon",
                             "/org/deepin/ai/daemon/EmbeddingPlatform",
                             "org.deepin.ai.daemon.EmbeddingPlatform",
                             QDBusConnection::sessionBus());
    QDBusPendingReply<QString> reply = interface.asyncCall("destroyIndex", appId, allIndex, extensionParams);
    reply.waitForFinished();
    if (reply.isError()) {
        qWarning() << "DBus error:" << reply.error().message();
        d->error = DTK_CORE_NAMESPACE::DError(AIErrorCode::APIServerNotAvailable, reply.error().message());
        return false;
    }
    
    // Parse JSON response to extract success field
    QString response = reply.value();
    QJsonDocument doc = QJsonDocument::fromJson(response.toUtf8());
    if (!doc.isObject()) {
        qWarning() << "Invalid JSON response:" << response;
        d->error = DTK_CORE_NAMESPACE::DError(AIErrorCode::APIServerNotAvailable, "Invalid JSON response");
        return false;
    }
    
    QJsonObject obj = doc.object();
    if (!obj.contains("success") || !obj["success"].isBool()) {
        qWarning() << "Missing or invalid success field in response:" << response;
        d->error = DTK_CORE_NAMESPACE::DError(AIErrorCode::APIServerNotAvailable, "Missing or invalid success field in response");
        return false;
    }
    
    d->error = DTK_CORE_NAMESPACE::DError(NoError, "");
    return obj["success"].toBool();
}

DTK_CORE_NAMESPACE::DError DEmbeddingPlatform::lastError() const
{
    D_DC(DEmbeddingPlatform);
    return d->error;
}

DAI_END_NAMESPACE
