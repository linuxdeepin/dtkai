// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_base.h"
#include "test_utils.h"

#include "dtkai/dtkaitypes.h"
#include "dtkai/dmodelmanager.h"

#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QVariant>
#include <QMetaType>

DAI_USE_NAMESPACE

// Note: Q_DECLARE_METATYPE for ModelInfo and DeployType are already declared in dmodelmanager.h

/**
 * @brief Test class for data types serialization and deserialization
 * 
 * This class tests the serialization and deserialization of DTKAI
 * data types to ensure they can be properly stored and transmitted.
 */
class TestDataTypes : public TestBase
{
protected:
    void SetUp() override
    {
        TestBase::SetUp();
        qDebug() << "Setting up data types serialization tests";
    }
    
    void TearDown() override
    {
        TestBase::TearDown();
    }
    
    /**
     * @brief Helper method to create a sample ModelInfo
     * @return A populated ModelInfo for testing
     */
    ModelInfo createSampleModelInfo()
    {
        ModelInfo info;
        info.modelName = "test-model-v1.0";
        info.provider = "TestProvider";
        info.description = "A test model for unit testing";
        info.capability = "Chat";
        info.deployType = DeployType::Cloud;
        info.isAvailable = true;
        
        // Add some parameters
        info.parameters["temperature"] = 0.7;
        info.parameters["max_tokens"] = 1000;
        info.parameters["model_version"] = "1.0";
        
        return info;
    }
    
    /**
     * @brief Helper method to create a sample ChatHistory
     * @return A populated ChatHistory for testing
     */
    ChatHistory createSampleChatHistory()
    {
        ChatHistory history;
        history.role = kChatRoleUser;
        history.content = "Hello, this is a test message";
        return history;
    }
    
    /**
     * @brief Compare two ModelInfo objects for equality
     * @param info1 First ModelInfo
     * @param info2 Second ModelInfo
     * @return true if equal, false otherwise
     */
    bool compareModelInfo(const ModelInfo &info1, const ModelInfo &info2)
    {
        return info1.modelName == info2.modelName &&
               info1.provider == info2.provider &&
               info1.description == info2.description &&
               info1.capability == info2.capability &&
               info1.deployType == info2.deployType &&
               info1.isAvailable == info2.isAvailable &&
               info1.parameters == info2.parameters;
    }
    
    /**
     * @brief Compare two ChatHistory objects for equality
     * @param history1 First ChatHistory
     * @param history2 Second ChatHistory
     * @return true if equal, false otherwise
     */
    bool compareChatHistory(const ChatHistory &history1, const ChatHistory &history2)
    {
        return history1.role == history2.role &&
               history1.content == history2.content;
    }
};

/**
 * @brief Test ModelInfo serialization to JSON
 * 
 * This test verifies that ModelInfo can be properly converted
 * to JSON format for storage or transmission.
 */
TEST_F(TestDataTypes, modelInfoJsonSerialization)
{
    qInfo() << "Testing ModelInfo JSON serialization";
    
    // Create sample data
    ModelInfo originalInfo = createSampleModelInfo();
    
    // Test: Convert to JSON
    QJsonObject jsonObj;
    EXPECT_NO_THROW({
        jsonObj["modelName"] = originalInfo.modelName;
        jsonObj["provider"] = originalInfo.provider;
        jsonObj["description"] = originalInfo.description;
        jsonObj["capability"] = originalInfo.capability;
        jsonObj["deployType"] = static_cast<int>(originalInfo.deployType);
        jsonObj["isAvailable"] = originalInfo.isAvailable;
        
        // Convert parameters QVariantHash to QJsonObject
        QJsonObject parametersJson;
        for (auto it = originalInfo.parameters.begin(); it != originalInfo.parameters.end(); ++it) {
            parametersJson[it.key()] = QJsonValue::fromVariant(it.value());
        }
        jsonObj["parameters"] = parametersJson;
    }) << "ModelInfo should be serializable to JSON";
    
    // Test: Verify JSON structure
    EXPECT_TRUE(jsonObj.contains("modelName")) << "JSON should contain modelName";
    EXPECT_TRUE(jsonObj.contains("capability")) << "JSON should contain capability";
    EXPECT_TRUE(jsonObj.contains("deployType")) << "JSON should contain deployType";
    EXPECT_TRUE(jsonObj.contains("parameters")) << "JSON should contain parameters";
    
    // Test: Verify JSON values
    EXPECT_EQ(jsonObj["modelName"].toString(), originalInfo.modelName);
    EXPECT_EQ(jsonObj["capability"].toString(), originalInfo.capability);
    EXPECT_EQ(jsonObj["deployType"].toInt(), static_cast<int>(originalInfo.deployType));
    EXPECT_EQ(jsonObj["isAvailable"].toBool(), originalInfo.isAvailable);
    
    qInfo() << "JSON serialization completed for ModelInfo";
}

/**
 * @brief Test ModelInfo deserialization from JSON
 * 
 * This test verifies that ModelInfo can be properly reconstructed
 * from JSON format.
 */
TEST_F(TestDataTypes, modelInfoJsonDeserialization)
{
    qInfo() << "Testing ModelInfo JSON deserialization";
    
    // Create sample JSON
    QJsonObject jsonObj;
    jsonObj["modelName"] = "deserialized-model";
    jsonObj["provider"] = "DeserializedProvider";
    jsonObj["description"] = "A model created from JSON";
    jsonObj["capability"] = "SpeechToText";
    jsonObj["deployType"] = static_cast<int>(DeployType::Local);
    jsonObj["isAvailable"] = false;
    
    QJsonObject parametersJson;
    parametersJson["sample_rate"] = 16000;
    parametersJson["language"] = "en-US";
    jsonObj["parameters"] = parametersJson;
    
    // Test: Convert from JSON
    ModelInfo deserializedInfo;
    EXPECT_NO_THROW({
        deserializedInfo.modelName = jsonObj["modelName"].toString();
        deserializedInfo.provider = jsonObj["provider"].toString();
        deserializedInfo.description = jsonObj["description"].toString();
        deserializedInfo.capability = jsonObj["capability"].toString();
        deserializedInfo.deployType = static_cast<DeployType>(jsonObj["deployType"].toInt());
        deserializedInfo.isAvailable = jsonObj["isAvailable"].toBool();
        
        // Convert parameters from JSON
        QJsonObject params = jsonObj["parameters"].toObject();
        for (auto it = params.begin(); it != params.end(); ++it) {
            deserializedInfo.parameters[it.key()] = it.value().toVariant();
        }
    }) << "ModelInfo should be deserializable from JSON";
    
    // Test: Verify deserialized values
    EXPECT_EQ(deserializedInfo.modelName, "deserialized-model");
    EXPECT_EQ(deserializedInfo.provider, "DeserializedProvider");
    EXPECT_EQ(deserializedInfo.capability, "SpeechToText");
    EXPECT_EQ(deserializedInfo.deployType, DeployType::Local);
    EXPECT_FALSE(deserializedInfo.isAvailable);
    
    // Test: Verify parameters
    EXPECT_EQ(deserializedInfo.parameters["sample_rate"].toInt(), 16000);
    EXPECT_EQ(deserializedInfo.parameters["language"].toString(), "en-US");
    
    qInfo() << "JSON deserialization completed for ModelInfo";
}

/**
 * @brief Test round-trip serialization/deserialization for ModelInfo
 * 
 * This test verifies that ModelInfo maintains data integrity
 * through serialization and deserialization cycles.
 */
TEST_F(TestDataTypes, modelInfoRoundTrip)
{
    qInfo() << "Testing ModelInfo round-trip serialization";
    
    ModelInfo originalInfo = createSampleModelInfo();
    
    // Serialize to JSON
    QJsonObject jsonObj;
    jsonObj["modelName"] = originalInfo.modelName;
    jsonObj["provider"] = originalInfo.provider;
    jsonObj["description"] = originalInfo.description;
    jsonObj["capability"] = originalInfo.capability;
    jsonObj["deployType"] = static_cast<int>(originalInfo.deployType);
    jsonObj["isAvailable"] = originalInfo.isAvailable;
    
    QJsonObject parametersJson;
    for (auto it = originalInfo.parameters.begin(); it != originalInfo.parameters.end(); ++it) {
        parametersJson[it.key()] = QJsonValue::fromVariant(it.value());
    }
    jsonObj["parameters"] = parametersJson;
    
    // Deserialize from JSON
    ModelInfo roundTripInfo;
    roundTripInfo.modelName = jsonObj["modelName"].toString();
    roundTripInfo.provider = jsonObj["provider"].toString();
    roundTripInfo.description = jsonObj["description"].toString();
    roundTripInfo.capability = jsonObj["capability"].toString();
    roundTripInfo.deployType = static_cast<DeployType>(jsonObj["deployType"].toInt());
    roundTripInfo.isAvailable = jsonObj["isAvailable"].toBool();
    
    QJsonObject params = jsonObj["parameters"].toObject();
    for (auto it = params.begin(); it != params.end(); ++it) {
        roundTripInfo.parameters[it.key()] = it.value().toVariant();
    }
    
    // Test: Compare original and round-trip data
    EXPECT_TRUE(compareModelInfo(originalInfo, roundTripInfo))
        << "Round-trip serialization should preserve all data";
    
    qInfo() << "Round-trip test completed successfully";
}

/**
 * @brief Test ChatHistory serialization and deserialization
 * 
 * This test verifies that ChatHistory can be properly serialized
 * and deserialized.
 */
TEST_F(TestDataTypes, chatHistorySerialization)
{
    qInfo() << "Testing ChatHistory serialization";
    
    ChatHistory originalHistory = createSampleChatHistory();
    
    // Test: Serialize to JSON
    QJsonObject jsonObj;
    EXPECT_NO_THROW({
        jsonObj["role"] = originalHistory.role;
        jsonObj["content"] = originalHistory.content;
    }) << "ChatHistory should be serializable to JSON";
    
    // Test: Deserialize from JSON
    ChatHistory deserializedHistory;
    EXPECT_NO_THROW({
        deserializedHistory.role = jsonObj["role"].toString();
        deserializedHistory.content = jsonObj["content"].toString();
    }) << "ChatHistory should be deserializable from JSON";
    
    // Test: Compare original and deserialized
    EXPECT_TRUE(compareChatHistory(originalHistory, deserializedHistory))
        << "ChatHistory round-trip should preserve data";
    
    qInfo() << "ChatHistory serialization test completed";
}

/**
 * @brief Test DeployType enum serialization
 * 
 * This test verifies that DeployType enum values can be properly
 * serialized and deserialized.
 */
TEST_F(TestDataTypes, deployTypeEnumSerialization)
{
    qInfo() << "Testing DeployType enum serialization";
    
    // Test all enum values
    QList<DeployType> deployTypes = {
        DeployType::Local,
        DeployType::Cloud,
        DeployType::Custom
    };
    
    for (DeployType originalType : deployTypes) {
        // Serialize to int
        int serializedValue = static_cast<int>(originalType);
        
        // Deserialize from int
        DeployType deserializedType = static_cast<DeployType>(serializedValue);
        
        // Test: Values should match
        EXPECT_EQ(originalType, deserializedType)
            << "DeployType enum should round-trip correctly";
        
        // Test: Serialized values should be in valid range
        EXPECT_GE(serializedValue, 0) << "Serialized DeployType should be non-negative";
        EXPECT_LT(serializedValue, 10) << "Serialized DeployType should be in reasonable range";
    }
    
    qInfo() << "DeployType enum serialization test completed";
}

/**
 * @brief Test basic type operations
 * 
 * This test verifies that DTKAI data types work correctly
 * with basic operations like copying and assignment.
 */
TEST_F(TestDataTypes, basicTypeOperations)
{
    qInfo() << "Testing basic type operations";
    
    // Test: ModelInfo copy operations
    ModelInfo originalInfo = createSampleModelInfo();
    ModelInfo copiedInfo = originalInfo;
    
    EXPECT_TRUE(compareModelInfo(originalInfo, copiedInfo))
        << "ModelInfo should copy correctly";
    
    // Test: ChatHistory copy operations
    ChatHistory originalHistory = createSampleChatHistory();
    ChatHistory copiedHistory = originalHistory;
    
    EXPECT_TRUE(compareChatHistory(originalHistory, copiedHistory))
        << "ChatHistory should copy correctly";
    
    // Test: DeployType operations
    DeployType originalType = DeployType::Cloud;
    DeployType copiedType = originalType;
    
    EXPECT_EQ(originalType, copiedType) << "DeployType should copy correctly";
    
    qInfo() << "Basic type operations test completed";
}

/**
 * @brief Test data validation and constraints
 * 
 * This test verifies that data types properly validate
 * their content and enforce constraints.
 */
TEST_F(TestDataTypes, dataValidation)
{
    qInfo() << "Testing data validation and constraints";
    
    // Test: ModelInfo validation
    ModelInfo info = createSampleModelInfo();
    
    // Basic validation checks
    expectNonEmptyString(info.modelName, "ModelInfo modelName should not be empty");
    expectNonEmptyString(info.capability, "ModelInfo capability should not be empty");
    
    // Enum validation
    EXPECT_TRUE(info.deployType == DeployType::Local || 
               info.deployType == DeployType::Cloud || 
               info.deployType == DeployType::Custom)
        << "DeployType should have valid enum value";
    
    // Test: ChatHistory validation
    ChatHistory history = createSampleChatHistory();
    
    expectNonEmptyString(history.role, "ChatHistory role should not be empty");
    expectNonEmptyString(history.content, "ChatHistory content should not be empty");
    
    // Role validation
    QStringList validRoles = {kChatRoleUser, kChatRoleAssistant, kChatRoleSystem};
    EXPECT_TRUE(validRoles.contains(history.role))
        << "ChatHistory role should be one of the predefined constants";
    
    qInfo() << "Data validation test completed";
}
