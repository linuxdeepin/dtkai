// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_base.h"
#include "test_utils.h"

#include "dtkai/dtkai_global.h"
#include "dtkai/dtkaitypes.h"
#include "dtkai/dmodelmanager.h"

DAI_USE_NAMESPACE

/**
 * @brief 基础接口测试类
 * 
 * 测试dtkai的基础接口和数据类型
 */
class TestBasicInterfaces : public TestBase
{
protected:
    void SetUp() override
    {
        TestBase::SetUp();
        qDebug() << "Setting up basic interfaces test";
    }
};

/**
 * @brief 测试全局定义和宏
 */
TEST_F(TestBasicInterfaces, globalDefinitions)
{
    // Test: 验证命名空间宏定义存在
    // 这些是编译时检查，如果定义不存在会编译失败
    SUCCEED() << "DAI_NAMESPACE and related macros are defined";
}

/**
 * @brief 测试基础数据类型
 */
TEST_F(TestBasicInterfaces, basicDataTypes)
{
    // Test: 验证DeployType枚举
    // 注意：这里我们测试枚举值的存在性，不测试具体值
    EXPECT_NO_THROW({
        // 如果枚举定义存在，这些代码应该能编译通过
        int localType = static_cast<int>(DeployType::Local);
        int cloudType = static_cast<int>(DeployType::Cloud);
        int customType = static_cast<int>(DeployType::Custom);
        Q_UNUSED(localType)
        Q_UNUSED(cloudType)
        Q_UNUSED(customType)
    }) << "DeployType enum should be accessible";
}

/**
 * @brief 测试ModelInfo结构体
 */
TEST_F(TestBasicInterfaces, modelInfoStruct)
{
    // Test: 验证ModelInfo结构体可以创建和使用
    EXPECT_NO_THROW({
        ModelInfo info;
        
        // 测试基本属性访问
        info.modelName = "test-model";
        info.provider = "test-provider";
        info.description = "test-description";
        info.capability = "Chat";
        info.deployType = DeployType::Cloud;
        info.isAvailable = true;
        
        // 验证赋值成功
        EXPECT_EQ(info.modelName, "test-model");
        EXPECT_EQ(info.provider, "test-provider");
        EXPECT_EQ(info.description, "test-description");
        EXPECT_EQ(info.capability, "Chat");
        EXPECT_EQ(info.deployType, DeployType::Cloud);
        EXPECT_TRUE(info.isAvailable);
        
    }) << "ModelInfo struct should be usable";
}

/**
 * @brief 测试ChatHistory结构体
 */
TEST_F(TestBasicInterfaces, chatHistoryStruct)
{
    // Test: 验证ChatHistory结构体可以创建和使用
    EXPECT_NO_THROW({
        ChatHistory history;
        
        // 测试基本属性访问
        history.role = kChatRoleUser;
        history.content = "Hello, AI!";
        
        // 验证赋值成功
        EXPECT_EQ(history.role, kChatRoleUser);
        EXPECT_EQ(history.content, "Hello, AI!");
        
        // 测试角色常量
        EXPECT_STREQ(kChatRoleUser, "user");
        EXPECT_STREQ(kChatRoleAssistant, "assistant");
        EXPECT_STREQ(kChatRoleSystem, "system");
        
    }) << "ChatHistory struct should be usable";
}

/**
 * @brief 测试测试工具本身
 */
TEST_F(TestBasicInterfaces, testUtils)
{
    // Test: 验证测试工具类功能
    QString testPath = TestUtils::getResourcePath("test.txt");
    EXPECT_FALSE(testPath.isEmpty()) << "Resource path should be generated";
    EXPECT_TRUE(testPath.contains("resources")) << "Path should contain resources directory";
    
    // Test: 生成模拟数据
    QByteArray audioData = TestUtils::generateAudioData(16000, 1);
    EXPECT_FALSE(audioData.isEmpty()) << "Audio data should be generated";
    EXPECT_TRUE(TestUtils::isValidAudioFormat(audioData)) << "Generated audio should be valid";
    
    QByteArray imageData = TestUtils::generateImageData(100, 100);
    EXPECT_FALSE(imageData.isEmpty()) << "Image data should be generated";
    EXPECT_TRUE(TestUtils::isValidImageFormat(imageData)) << "Generated image should be valid";
}

/**
 * @brief 测试Qt环境
 */
TEST_F(TestBasicInterfaces, qtEnvironment)
{
    // Test: 验证Qt环境正常
    EXPECT_TRUE(QCoreApplication::instance() != nullptr) << "QCoreApplication should exist";
    
    // Test: 验证QString基本功能
    QString testStr = "DTKAI Test";
    EXPECT_EQ(testStr.length(), 10);
    EXPECT_TRUE(testStr.contains("DTKAI"));
    
    // Test: 验证QByteArray基本功能
    QByteArray testData = "Hello World";
    EXPECT_EQ(testData.size(), 11);
    EXPECT_TRUE(testData.contains("World"));
}
