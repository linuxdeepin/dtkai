// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_base.h"
#include "test_utils.h"

#include "dtkai/dmodelmanager.h"
#include "dtkai/daierror.h"

#include <thread>
#include <atomic>

DAI_USE_NAMESPACE

/**
 * @brief Test class for DModelManager interface
 * 
 * This class tests the static methods of DModelManager which provide
 * read-only access to AI model information and capabilities.
 */
class TestDModelManager : public TestBase
{
protected:
    void SetUp() override
    {
        TestBase::SetUp();
        qDebug() << "Setting up DModelManager interface tests";
    }
    
    void TearDown() override
    {
        TestBase::TearDown();
    }
    
    /**
     * @brief Helper method to validate ModelInfo structure
     * @param info The ModelInfo to validate
     * @param shouldBeValid Whether the info should be valid
     */
    void validateModelInfo(const ModelInfo &info, bool shouldBeValid = true)
    {
        if (shouldBeValid) {
            expectNonEmptyString(info.modelName, "ModelInfo.modelName should not be empty");
            expectNonEmptyString(info.capability, "ModelInfo.capability should not be empty");
            // provider and description can be empty for some models
            
            // Validate deployType enum
            EXPECT_TRUE(info.deployType == DeployType::Local || 
                       info.deployType == DeployType::Cloud || 
                       info.deployType == DeployType::Custom)
                << "DeployType should be valid enum value";
        }
    }
    
    /**
     * @brief Helper method to validate capability string
     * @param capability The capability string to validate
     */
    void validateCapability(const QString &capability)
    {
        expectNonEmptyString(capability, "Capability should not be empty");
        
        // Common capability types that should be supported
        QStringList knownCapabilities = {
            "Chat", "SpeechToText", "TextToSpeech", 
            "ImageRecognition", "OCR", "FunctionCalling"
        };
        
        // At least one known capability should be present in supported list
        bool hasKnownCapability = false;
        for (const QString &known : knownCapabilities) {
            if (capability == known) {
                hasKnownCapability = true;
                break;
            }
        }
        
        if (!hasKnownCapability) {
            qInfo() << "Found unknown capability:" << capability << "- this might be a new feature";
        }
    }
};

/**
 * @brief Test DModelManager::supportedCapabilities() static method
 * 
 * This test verifies that the supportedCapabilities method returns
 * a valid list of AI capabilities that the system supports.
 */
TEST_F(TestDModelManager, supportedCapabilities)
{
    qInfo() << "Testing DModelManager::supportedCapabilities()";
    
    // Test: Call the static method
    QStringList capabilities;
    EXPECT_NO_THROW({
        capabilities = DModelManager::supportedCapabilities();
    }) << "supportedCapabilities() should not throw exceptions";
    
    // Test: Verify result is not empty
    expectNonEmptyList(capabilities, "Supported capabilities list should not be empty");
    
    // Test: Validate each capability
    for (const QString &capability : capabilities) {
        validateCapability(capability);
    }
    
    // Test: Log capabilities for debugging
    qInfo() << "Supported capabilities:" << capabilities;
    
    // Test: Verify no duplicates
    QSet<QString> uniqueCapabilities = capabilities.toSet();
    EXPECT_EQ(capabilities.size(), uniqueCapabilities.size())
        << "Capabilities list should not contain duplicates";
}

/**
 * @brief Test DModelManager::isCapabilityAvailable() static method
 * 
 * This test verifies that the isCapabilityAvailable method correctly
 * reports whether specific capabilities are available.
 */
TEST_F(TestDModelManager, isCapabilityAvailable)
{
    qInfo() << "Testing DModelManager::isCapabilityAvailable()";
    
    // First get the supported capabilities
    QStringList supportedCapabilities = DModelManager::supportedCapabilities();
    ASSERT_FALSE(supportedCapabilities.isEmpty()) << "Need supported capabilities for this test";
    
    // Test: Check each supported capability
    for (const QString &capability : supportedCapabilities) {
        bool isAvailable = false;
        EXPECT_NO_THROW({
            isAvailable = DModelManager::isCapabilityAvailable(capability);
        }) << "isCapabilityAvailable() should not throw for valid capability: " << capability.toStdString();
        
        EXPECT_TRUE(isAvailable) 
            << "Supported capability should be available: " << capability.toStdString();
    }
    
    // Test: Check unsupported capability
    QString nonExistentCapability = "NonExistentCapability_" + QString::number(QDateTime::currentMSecsSinceEpoch());
    bool shouldBeFalse = false;
    EXPECT_NO_THROW({
        shouldBeFalse = DModelManager::isCapabilityAvailable(nonExistentCapability);
    }) << "isCapabilityAvailable() should not throw for non-existent capability";
    
    EXPECT_FALSE(shouldBeFalse) 
        << "Non-existent capability should not be available: " << nonExistentCapability.toStdString();
    
    // Test: Check empty string
    EXPECT_NO_THROW({
        shouldBeFalse = DModelManager::isCapabilityAvailable("");
    }) << "isCapabilityAvailable() should handle empty string gracefully";
    
    EXPECT_FALSE(shouldBeFalse) << "Empty capability string should not be available";
}

/**
 * @brief Test DModelManager::availableModels() static method
 * 
 * This test verifies that the availableModels method returns
 * valid model information for specific capabilities.
 */
TEST_F(TestDModelManager, availableModels)
{
    qInfo() << "Testing DModelManager::availableModels()";
    
    // Get supported capabilities first
    QStringList supportedCapabilities = DModelManager::supportedCapabilities();
    ASSERT_FALSE(supportedCapabilities.isEmpty()) << "Need supported capabilities for this test";
    
    // Test: Get models for each supported capability
    for (const QString &capability : supportedCapabilities) {
        QList<ModelInfo> models;
        EXPECT_NO_THROW({
            models = DModelManager::availableModels(capability);
        }) << "availableModels() should not throw for valid capability: " << capability.toStdString();
        
        qInfo() << "Found" << models.size() << "models for capability:" << capability;
        
        // Test: Validate each model
        for (const ModelInfo &model : models) {
            validateModelInfo(model, true);
            
            // Test: Model capability should match requested capability
            EXPECT_EQ(model.capability, capability)
                << "Model capability should match requested capability";
        }
    }
    
    // Test: Get all models
    QList<ModelInfo> allModels;
    EXPECT_NO_THROW({
        allModels = DModelManager::availableModels();
    }) << "availableModels() without parameter should not throw";
    
    qInfo() << "Total available models:" << allModels.size();
    
    // Test: Validate all models
    for (const ModelInfo &model : allModels) {
        validateModelInfo(model, true);
    }
    
    // Test: Invalid capability
    QList<ModelInfo> emptyModels;
    EXPECT_NO_THROW({
        emptyModels = DModelManager::availableModels("InvalidCapability123");
    }) << "availableModels() should handle invalid capability gracefully";
    
    // Empty result is acceptable for invalid capability
    qInfo() << "Models for invalid capability:" << emptyModels.size();
}

/**
 * @brief Test DModelManager::modelInfo() static method
 * 
 * This test verifies that the modelInfo method returns
 * detailed information for specific models.
 */
TEST_F(TestDModelManager, modelInfo)
{
    qInfo() << "Testing DModelManager::modelInfo()";
    
    // Get some models to test with
    QList<ModelInfo> allModels = DModelManager::availableModels();
    
    if (allModels.isEmpty()) {
        qWarning() << "No models available for testing modelInfo()";
        SUCCEED() << "No models available for testing - skipping model-specific tests";
    }
    
    // Test: Get info for each available model
    for (const ModelInfo &model : allModels) {
        if (model.modelName.isEmpty()) continue;
        
        ModelInfo retrievedInfo;
        EXPECT_NO_THROW({
            retrievedInfo = DModelManager::modelInfo(model.modelName);
        }) << "modelInfo() should not throw for valid model: " << model.modelName.toStdString();
        
        // Test: Retrieved info should match original
        EXPECT_EQ(retrievedInfo.modelName, model.modelName)
            << "Retrieved model name should match";
        EXPECT_EQ(retrievedInfo.capability, model.capability)
            << "Retrieved capability should match";
        EXPECT_EQ(retrievedInfo.deployType, model.deployType)
            << "Retrieved deploy type should match";
        
        validateModelInfo(retrievedInfo, true);
        
        qInfo() << "Model info for" << model.modelName << "- Provider:" << retrievedInfo.provider;
    }
    
    // Test: Invalid model name
    QString invalidModelName = "InvalidModel_" + QString::number(QDateTime::currentMSecsSinceEpoch());
    ModelInfo invalidInfo;
    EXPECT_NO_THROW({
        invalidInfo = DModelManager::modelInfo(invalidModelName);
    }) << "modelInfo() should handle invalid model name gracefully";
    
    // For invalid model, the result might be empty or have empty modelName
    qInfo() << "Info for invalid model - ModelName:" << invalidInfo.modelName;
    
    // Test: Empty model name
    EXPECT_NO_THROW({
        invalidInfo = DModelManager::modelInfo("");
    }) << "modelInfo() should handle empty model name gracefully";
}

/**
 * @brief Test error handling and edge cases
 * 
 * This test verifies that DModelManager methods handle
 * various error conditions gracefully.
 */
TEST_F(TestDModelManager, errorHandling)
{
    qInfo() << "Testing DModelManager error handling";
    
    // Test: All methods should be callable multiple times
    for (int i = 0; i < 3; ++i) {
        EXPECT_NO_THROW({
            auto caps = DModelManager::supportedCapabilities();
            if (!caps.isEmpty()) {
                DModelManager::isCapabilityAvailable(caps.first());
                auto models = DModelManager::availableModels(caps.first());
                if (!models.isEmpty()) {
                    DModelManager::modelInfo(models.first().modelName);
                }
            }
        }) << "Multiple calls should work correctly (iteration " << i << ")";
    }
    
    // Test: Thread safety (basic check)
    // Note: This is a simple check, not a comprehensive thread safety test
    std::atomic<int> successCount(0);
    std::vector<std::thread> threads;
    
    for (int i = 0; i < 3; ++i) {
        threads.emplace_back([&successCount]() {
            try {
                auto caps = DModelManager::supportedCapabilities();
                if (!caps.isEmpty()) {
                    DModelManager::isCapabilityAvailable(caps.first());
                }
                successCount++;
            } catch (...) {
                // Thread failed
            }
        });
    }
    
    for (auto &thread : threads) {
        thread.join();
    }
    
    EXPECT_EQ(successCount.load(), 3) << "All threads should succeed in basic operations";
    
    qInfo() << "Error handling tests completed successfully";
}
