// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DMODELMANAGER_H
#define DMODELMANAGER_H

#include "dtkai_global.h"
#include "dtkaitypes.h"

#include <QStringList>
#include <QVariantHash>

DAI_BEGIN_NAMESPACE

enum class DeployType {
    Local,      // Local model
    Cloud,      // Cloud model
    Custom      // Custom model
};

struct ModelInfo {
    QString modelName;
    QString provider;
    QString description;
    QString capability; // "Chat", "SpeechToText" etc.
    DeployType deployType;
    bool isAvailable = false;
    QVariantHash parameters;  // Model-specific default parameters
};

class DModelManager
{
public:
    // Get supported capabilities
    // return "Chat", "SpeechToText" etc.
    static QStringList supportedCapabilities();
    static bool isCapabilityAvailable(const QString &capability);

    // Static query interfaces - read-only model information
    // capability is query from supportedCapabilities.
    static QList<ModelInfo> availableModels(const QString &capability);
    static QList<ModelInfo> availableModels(); // All models
    static ModelInfo modelInfo(const QString &modelName);

private:
    // Private constructor - static class only
    explicit DModelManager() = delete;
    ~DModelManager() = delete;
    DModelManager(const DModelManager&) = delete;
    DModelManager& operator=(const DModelManager&) = delete;
};

DAI_END_NAMESPACE

// Make enum available for QVariant
Q_DECLARE_METATYPE(DAI_NAMESPACE::DeployType)
Q_DECLARE_METATYPE(DAI_NAMESPACE::ModelInfo)

#endif // DMODELMANAGER_H
