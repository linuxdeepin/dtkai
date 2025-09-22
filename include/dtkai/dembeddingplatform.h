// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DEMBEDDINGPLATFORM_H
#define DEMBEDDINGPLATFORM_H

#include <dtkai_global.h>

#include <DObject>
#include <DError>

#include <QObject>
#include <QString>
#include <QStringList>
#include <QDateTime>
#include <QVariantMap>
#include <optional>

DAI_BEGIN_NAMESPACE

class DEmbeddingPlatformPrivate;
class DEmbeddingPlatform : public QObject, public Dtk::Core::DObject
{
    Q_OBJECT
    D_DECLARE_PRIVATE(DEmbeddingPlatform)

public:
    // Document information structure
    struct DocumentInfo {
        QString id;
        QString filePath;
        std::optional<QDateTime> createdAt;
        QVariantMap metadata;
        
        DocumentInfo() = default;
        DocumentInfo(const QString &id, const QString &filePath, const std::optional<QDateTime> &createdAt = std::nullopt, const QVariantMap &metadata = QVariantMap())
            : id(id), filePath(filePath), createdAt(createdAt), metadata(metadata) {}
    };
    
    // Search result structure
    struct SearchResult {
        struct Chunk {
            int chunkIndex;
            QString content;
            int tokens;
            QStringList timestamp;
            
            Chunk() = default;
            Chunk(int index, const QString &content, int tokens, const QStringList &timestamp)
                : chunkIndex(index), content(content), tokens(tokens), timestamp(timestamp) {}
        };
        
        Chunk chunk;
        double distance;
        QString id;
        QString model;
        
        SearchResult() = default;
        SearchResult(const Chunk &chunk, double distance, const QString &id, const QString &model)
            : chunk(chunk), distance(distance), id(id), model(model) {}
    };
    
    explicit DEmbeddingPlatform(QObject *parent = nullptr);
    ~DEmbeddingPlatform();

    QString embeddingModels();
    QList<DocumentInfo> uploadDocuments(const QString &appId, const QStringList &files, const QString &extensionParams = QString());
    bool deleteDocuments(const QString &appId, const QStringList &documentIds);
    QList<SearchResult> search(const QString &appId, const QString &query, const QString &extensionParams = QString());
    // TODO: taskId not found.
    bool cancelTask(const QString &taskId);
    QList<DocumentInfo> documentsInfo(const QString &appId, const QStringList &documentIds = {});
    bool buildIndex(const QString &appId, const QString &docId, const QString &extensionParams = QString());
    bool destroyIndex(const QString &appId, bool allIndex, const QString &extensionParams = QString());

    DTK_CORE_NAMESPACE::DError lastError() const;
};

DAI_END_NAMESPACE

#endif // DEMBEDDINGPLATFORM_H