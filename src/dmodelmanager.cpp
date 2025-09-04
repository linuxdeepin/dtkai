// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dmodelmanager.h"
#include "aidaemon_modelinfo.h" // D-Bus generated proxy header

#include <QDBusConnection>
#include <QDBusReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(dtkaiModelManager, "dtkai.modelmanager")

DAI_USE_NAMESPACE

namespace {
    // Helper function to get D-Bus interface
    OrgDeepinAiDaemonModelInfoInterface* getModelInfoInterface() {
        static OrgDeepinAiDaemonModelInfoInterface* interface = nullptr;
        if (!interface) {
            interface = new OrgDeepinAiDaemonModelInfoInterface(
                "org.deepin.ai.daemon.ModelInfo",
                "/org/deepin/ai/daemon/ModelInfo",
                QDBusConnection::sessionBus()
            );
        }
        return interface;
    }

    // Helper function to convert single JSON object to ModelInfo
    ModelInfo parseModelFromJson(const QString &jsonStr) {
        ModelInfo info;

        QJsonDocument doc = QJsonDocument::fromJson(jsonStr.toUtf8());
        if (!doc.isObject()) {
            qCWarning(dtkaiModelManager) << "Invalid JSON response for single model";
            return info;
        }

        QJsonObject modelObj = doc.object();
        if (modelObj.isEmpty()) {
            return info; // Empty model info for not found
        }

        info.modelName = modelObj["name"].toString();
        info.provider = modelObj["provider"].toString();
        info.description = modelObj["description"].toString();
        info.capability = modelObj["capability"].toString();
        info.isAvailable = modelObj["isAvailable"].toBool();

        // Parse deploy type
        QString deployTypeStr = modelObj["deployType"].toString();
        if (deployTypeStr == "Local") {
            info.deployType = DeployType::Local;
        } else if (deployTypeStr == "Cloud") {
            info.deployType = DeployType::Cloud;
        } else {
            info.deployType = DeployType::Custom;
        }

        // Parse parameters
        QJsonObject paramsObj = modelObj["parameters"].toObject();
        for (auto it = paramsObj.begin(); it != paramsObj.end(); ++it) {
            info.parameters[it.key()] = it.value().toVariant();
        }

        return info;
    }

    // Helper function to convert JSON string to ModelInfo list
    QList<ModelInfo> parseModelsFromJson(const QString &jsonStr) {
        QList<ModelInfo> models;

        QJsonDocument doc = QJsonDocument::fromJson(jsonStr.toUtf8());
        if (!doc.isObject()) {
            qCWarning(dtkaiModelManager) << "Invalid JSON response from daemon";
            return models;
        }

        QJsonObject root = doc.object();
        QJsonArray modelsArray = root["models"].toArray();

        for (const QJsonValue &value : modelsArray) {
            QJsonObject modelObj = value.toObject();
            
            // Convert single model object to JSON string and parse using existing function
            QJsonDocument modelDoc(modelObj);
            QString modelJsonStr = modelDoc.toJson(QJsonDocument::Compact);
            ModelInfo info = parseModelFromJson(modelJsonStr);
            
            if (!info.modelName.isEmpty()) {
                models.append(info);
            }
        }

        return models;
    }
}

QStringList DModelManager::supportedCapabilities()
{
    auto interface = getModelInfoInterface();
    if (!interface || !interface->isValid()) {
        qCWarning(dtkaiModelManager) << "ModelInfo D-Bus interface not available, using fallback";
        return {};
    }

    QDBusReply<QString> reply = interface->GetSupportedCapabilities();
    if (!reply.isValid()) {
        qCWarning(dtkaiModelManager) << "Failed to get supported capabilities:" << reply.error().message();
        return {};
    }

    // Parse JSON array response
    QJsonDocument doc = QJsonDocument::fromJson(reply.value().toUtf8());
    if (!doc.isArray()) {
        qCWarning(dtkaiModelManager) << "Invalid capabilities response format";
        return {};
    }

    QStringList capabilities;
    QJsonArray array = doc.array();
    for (const QJsonValue &value : array) {
        capabilities.append(value.toString());
    }

    qCDebug(dtkaiModelManager) << "Retrieved supported capabilities:" << capabilities;
    return capabilities;
}

bool DModelManager::isCapabilityAvailable(const QString &capability)
{
    // Check if any models support this capability
    auto models = availableModels(capability);
    return !models.isEmpty();
}

QList<ModelInfo> DModelManager::availableModels(const QString &capability)
{
    auto interface = getModelInfoInterface();
    if (!interface || !interface->isValid()) {
        qCWarning(dtkaiModelManager) << "ModelInfo D-Bus interface not available";
        return QList<ModelInfo>();
    }

    QDBusReply<QString> reply = interface->GetModelsForCapability(capability);
    if (!reply.isValid()) {
        qCWarning(dtkaiModelManager) << "Failed to get models for capability" << capability
                                    << ":" << reply.error().message();
        return QList<ModelInfo>();
    }

    QList<ModelInfo> models = parseModelsFromJson(reply.value());
    qCDebug(dtkaiModelManager) << "Found" << models.size() << "models for capability" << capability;
    return models;
}

QList<ModelInfo> DModelManager::availableModels()
{
    auto interface = getModelInfoInterface();
    if (!interface || !interface->isValid()) {
        qCWarning(dtkaiModelManager) << "ModelInfo D-Bus interface not available";
        return QList<ModelInfo>();
    }

    QDBusReply<QString> reply = interface->GetAllModels();
    if (!reply.isValid()) {
        qCWarning(dtkaiModelManager) << "Failed to get all models:" << reply.error().message();
        return QList<ModelInfo>();
    }

    QList<ModelInfo> models = parseModelsFromJson(reply.value());
    qCDebug(dtkaiModelManager) << "Found" << models.size() << "total models";
    return models;
}

ModelInfo DModelManager::modelInfo(const QString &modelName)
{
    auto interface = getModelInfoInterface();
    if (!interface || !interface->isValid()) {
        qCWarning(dtkaiModelManager) << "ModelInfo D-Bus interface not available";
        return ModelInfo();
    }

    QDBusReply<QString> reply = interface->GetModelInfo(modelName);
    if (!reply.isValid()) {
        qCWarning(dtkaiModelManager) << "Failed to get model info for" << modelName
                                    << ":" << reply.error().message();
        return ModelInfo();
    }

    ModelInfo info = parseModelFromJson(reply.value());
    if (info.modelName.isEmpty()) {
        qCWarning(dtkaiModelManager) << "Model not found:" << modelName;
    } else {
        qCDebug(dtkaiModelManager) << "Found model info for" << modelName;
    }

    return info;
}
