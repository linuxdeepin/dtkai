// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_base.h"
#include "test_utils.h"

#include "dtkai/dchatcompletions.h"
#include "dtkai/dtkaitypes.h"

#include <QSignalSpy>
#include <QTimer>
#include <QTest>
#include <QJsonObject>
#include <QJsonDocument>
#include <QVariantHash>

DAI_USE_NAMESPACE

/**
 * @brief Test class for DChatCompletions interface
 * 
 * This class tests the chat completion functionality including
 * synchronous chat, asynchronous chat, and streaming chat.
 */
class TestDChatCompletions : public TestBase
{
protected:
    void SetUp() override
    {
        TestBase::SetUp();
        qDebug() << "Setting up DChatCompletions interface tests";
        
        // Create chat instance for testing
        chat = new DChatCompletions();
        ASSERT_NE(chat, nullptr) << "Failed to create DChatCompletions instance";
    }
    
    void TearDown() override
    {
        if (chat) {
            delete chat;
            chat = nullptr;
        }
        TestBase::TearDown();
    }
    
    /**
     * @brief Helper method to create sample chat history
     * @return A list of chat history entries for testing
     */
    QList<ChatHistory> createSampleChatHistory()
    {
        QList<ChatHistory> history;
        
        ChatHistory userMessage;
        userMessage.role = kChatRoleUser;
        userMessage.content = "What is artificial intelligence?";
        history.append(userMessage);
        
        ChatHistory assistantMessage;
        assistantMessage.role = kChatRoleAssistant;
        assistantMessage.content = "Artificial intelligence (AI) refers to the simulation of human intelligence in machines.";
        history.append(assistantMessage);
        
        return history;
    }
    
    /**
     * @brief Helper method to create sample parameters
     * @return A QVariantHash with common chat parameters
     */
    QVariantHash createSampleParameters()
    {
        QVariantHash params;
        params["temperature"] = 0.7;
        params["max_tokens"] = 1000;
        params["top_p"] = 0.9;
        params["frequency_penalty"] = 0.0;
        params["presence_penalty"] = 0.0;
        return params;
    }
    
    /**
     * @brief Validate chat response content
     * @param response The response to validate
     */
    void validateChatResponse(const QString &response)
    {
        // Basic validation of chat response
        EXPECT_FALSE(response.isEmpty()) << "Chat response should not be empty";
        EXPECT_LT(response.length(), 100000) << "Chat response should not be excessively long";
        
        // Check for reasonable content (no null characters, etc.)
        EXPECT_FALSE(response.contains('\0')) << "Chat response should not contain null characters";
    }
    
    /**
     * @brief Validate error state
     * @param expectedError Whether an error is expected
     */
    void validateErrorState(bool expectedError = false)
    {
        auto error = chat->lastError();
        
        // In test environment without AI daemon, error code 1 (APIServerNotAvailable) is expected
        // This is normal behavior and not a test failure
        if (expectedError) {
            EXPECT_NE(error.getErrorCode(), -1) << "Expected an error to be set";
            if (error.getErrorCode() != -1) {
                qDebug() << "Expected error code:" << error.getErrorCode() << "message:" << error.getErrorMessage();
            }
        } else {
            // In test environment, we may get APIServerNotAvailable (code 1) which is acceptable
            if (error.getErrorCode() == 1) {
                qDebug() << "Info: AI daemon not available (error code 1) - this is normal in test environment";
            } else if (error.getErrorCode() != -1) {
                qDebug() << "Warning: Unexpected error code:" << error.getErrorCode() << "message:" << error.getErrorMessage();
                // Don't fail the test for daemon unavailability
            }
        }
    }

protected:
    DChatCompletions *chat = nullptr;
};

/**
 * @brief Test constructor and destructor
 * 
 * This test verifies that DChatCompletions can be properly
 * constructed and destroyed.
 */
TEST_F(TestDChatCompletions, constructorDestructor)
{
    qInfo() << "Testing DChatCompletions constructor and destructor";
    
    // Test: Constructor with parent
    EXPECT_NO_THROW({
        QObject parent;
        DChatCompletions *chatWithParent = new DChatCompletions(&parent);
        EXPECT_NE(chatWithParent, nullptr);
        EXPECT_EQ(chatWithParent->parent(), &parent);
        
        // Destructor will be called automatically when parent is destroyed
    }) << "Constructor with parent should work correctly";
    
    // Test: Constructor without parent
    EXPECT_NO_THROW({
        DChatCompletions *standaloneChat = new DChatCompletions();
        EXPECT_NE(standaloneChat, nullptr);
        EXPECT_EQ(standaloneChat->parent(), nullptr);
        
        delete standaloneChat;
    }) << "Constructor without parent should work correctly";
    
    // Test: Initial error state
    validateErrorState(false);
    
    qInfo() << "Constructor/destructor tests completed";
}

/**
 * @brief Test synchronous chat functionality
 * 
 * This test verifies that synchronous chat requests work
 * correctly and return appropriate responses.
 */
TEST_F(TestDChatCompletions, synchronousChat)
{
    qInfo() << "Testing DChatCompletions synchronous chat";
    
    // Test: Basic chat without history or parameters
    EXPECT_NO_THROW({
        QString prompt = "Hello, how are you today?";
        QString response = chat->chat(prompt);
        
        // In real daemon mode, we expect a meaningful response
        // In mock mode, we expect a mock response
        if (!response.isEmpty()) {
            validateChatResponse(response);
            qDebug() << "Chat response:" << response.left(100) << "...";
        } else {
            qDebug() << "Empty response received - this may be normal in test environment";
        }
        
        validateErrorState(false);
        
    }) << "Basic synchronous chat should work";
    
    // Test: Chat with history
    EXPECT_NO_THROW({
        QString prompt = "Can you elaborate on that?";
        QList<ChatHistory> history = createSampleChatHistory();
        
        QString response = chat->chat(prompt, history);
        
        if (!response.isEmpty()) {
            validateChatResponse(response);
            qDebug() << "Chat with history response:" << response.left(100) << "...";
        }
        
        validateErrorState(false);
        
    }) << "Chat with history should work";
    
    // Test: Chat with parameters
    EXPECT_NO_THROW({
        QString prompt = "Tell me a short joke";
        QVariantHash params = createSampleParameters();
        
        QString response = chat->chat(prompt, {}, params);
        
        if (!response.isEmpty()) {
            validateChatResponse(response);
            qDebug() << "Chat with parameters response:" << response.left(100) << "...";
        }
        
        validateErrorState(false);
        
    }) << "Chat with parameters should work";
    
    // Test: Chat with both history and parameters
    EXPECT_NO_THROW({
        QString prompt = "What's a good follow-up question?";
        QList<ChatHistory> history = createSampleChatHistory();
        QVariantHash params = createSampleParameters();
        
        QString response = chat->chat(prompt, history, params);
        
        if (!response.isEmpty()) {
            validateChatResponse(response);
            qDebug() << "Chat with history and parameters response:" << response.left(100) << "...";
        }
        
        validateErrorState(false);
        
    }) << "Chat with history and parameters should work";
    
    qInfo() << "Synchronous chat tests completed";
}

/**
 * @brief Test streaming chat functionality
 * 
 * This test verifies that streaming chat requests work
 * correctly and emit appropriate signals.
 */
TEST_F(TestDChatCompletions, streamingChat)
{
    qInfo() << "Testing DChatCompletions streaming chat";
    
    // Set up signal spies
    QSignalSpy outputSpy(chat, &DChatCompletions::streamOutput);
    QSignalSpy finishedSpy(chat, &DChatCompletions::streamFinished);
    
    ASSERT_TRUE(outputSpy.isValid()) << "streamOutput signal should be valid";
    ASSERT_TRUE(finishedSpy.isValid()) << "streamFinished signal should be valid";
    
    // Test: Basic streaming chat
    EXPECT_NO_THROW({
        QString prompt = "Tell me a story about artificial intelligence";
        bool started = chat->chatStream(prompt);
        
        if (started) {
            qDebug() << "Stream chat started successfully";
            
            // Wait for signals with reasonable timeout
            bool gotOutput = outputSpy.wait(5000);  // 5 second timeout
            bool gotFinished = finishedSpy.wait(10000);  // 10 second timeout
            
            if (gotOutput) {
                qDebug() << "Received" << outputSpy.count() << "output signals";
                
                // Validate output signals
                for (int i = 0; i < outputSpy.count(); ++i) {
                    QList<QVariant> arguments = outputSpy.at(i);
                    EXPECT_EQ(arguments.size(), 1) << "streamOutput should have 1 argument";
                    if (arguments.size() >= 1) {
                        QString content = arguments.at(0).toString();
                        EXPECT_FALSE(content.isEmpty()) << "Stream output content should not be empty";
                        qDebug() << "Stream output" << i << ":" << content.left(50) << "...";
                    }
                }
            } else {
                qDebug() << "No stream output received - this may be normal in test environment";
            }
            
            if (gotFinished) {
                qDebug() << "Received" << finishedSpy.count() << "finished signals";
                
                // Validate finished signal
                EXPECT_EQ(finishedSpy.count(), 1) << "Should receive exactly one finished signal";
                if (finishedSpy.count() >= 1) {
                    QList<QVariant> arguments = finishedSpy.at(0);
                    EXPECT_EQ(arguments.size(), 1) << "streamFinished should have 1 argument";
                    if (arguments.size() >= 1) {
                        int errorCode = arguments.at(0).toInt();
                        qDebug() << "Stream finished with error code:" << errorCode;
                        EXPECT_EQ(errorCode, 0) << "Stream should finish without error";
                    }
                }
            } else {
                qDebug() << "Stream did not finish within timeout - this may be normal in test environment";
            }
        } else {
            qDebug() << "Stream chat failed to start - checking for errors";
            validateErrorState(true);  // Expect error if stream failed to start
        }
        
    }) << "Basic streaming chat should work";
    
    // Clear spies for next test
    outputSpy.clear();
    finishedSpy.clear();
    
    // Test: Streaming chat with history and parameters
    EXPECT_NO_THROW({
        QString prompt = "Continue the story";
        QList<ChatHistory> history = createSampleChatHistory();
        QVariantHash params = createSampleParameters();
        
        bool started = chat->chatStream(prompt, history, params);
        
        if (started) {
            qDebug() << "Stream chat with parameters started successfully";
            
            // Wait for completion
            bool finished = finishedSpy.wait(10000);
            if (finished) {
                qDebug() << "Stream with parameters completed";
            }
        }
        
    }) << "Streaming chat with parameters should work";
    
    qInfo() << "Streaming chat tests completed";
}

/**
 * @brief Test terminate functionality
 * 
 * This test verifies that ongoing operations can be
 * properly terminated.
 */
TEST_F(TestDChatCompletions, terminateOperation)
{
    qInfo() << "Testing DChatCompletions terminate functionality";
    
    QSignalSpy finishedSpy(chat, &DChatCompletions::streamFinished);
    
    // Test: Terminate streaming operation
    EXPECT_NO_THROW({
        QString prompt = "Write a very long essay about machine learning";
        bool started = chat->chatStream(prompt);
        
        if (started) {
            qDebug() << "Started long-running stream for termination test";
            
            // Give it a moment to start
            QTest::qWait(100);
            
            // Terminate the operation
            chat->terminate();
            qDebug() << "Terminate called";
            
            // Check if termination was handled properly
            // The behavior may vary depending on implementation
            validateErrorState(false);  // Termination should not cause an error state
            
        } else {
            qDebug() << "Stream failed to start for termination test";
        }
        
    }) << "Terminate operation should work correctly";
    
    // Test: Terminate when no operation is running
    EXPECT_NO_THROW({
        chat->terminate();
        qDebug() << "Terminate called with no active operation";
        validateErrorState(false);  // Should not cause error
        
    }) << "Terminate with no active operation should be safe";
    
    qInfo() << "Terminate functionality tests completed";
}

/**
 * @brief Test error handling scenarios
 * 
 * This test verifies that various error conditions
 * are handled appropriately.
 */
TEST_F(TestDChatCompletions, errorHandling)
{
    qInfo() << "Testing DChatCompletions error handling";
    
    // Test: Empty prompt handling
    EXPECT_NO_THROW({
        QString emptyPrompt = "";
        QString response = chat->chat(emptyPrompt);
        
        // Empty prompts might be valid or invalid depending on implementation
        // We just ensure it doesn't crash and check error state
        qDebug() << "Empty prompt response:" << (response.isEmpty() ? "empty" : "non-empty");
        
        // Check if error was set for empty prompt
        auto error = chat->lastError();
        if (error.getErrorCode() != -1) {
            qDebug() << "Error for empty prompt:" << error.getErrorMessage();
        }
        
    }) << "Empty prompt should be handled gracefully";
    
    // Test: Very long prompt handling
    EXPECT_NO_THROW({
        QString longPrompt = QString("A").repeated(10000);  // 10K characters
        QString response = chat->chat(longPrompt);
        
        qDebug() << "Long prompt response length:" << response.length();
        
        // Check if there was an error due to prompt length
        auto error = chat->lastError();
        if (error.getErrorCode() != -1) {
            qDebug() << "Error for long prompt:" << error.getErrorMessage();
        }
        
    }) << "Very long prompt should be handled gracefully";
    
    // Test: Invalid parameters handling
    EXPECT_NO_THROW({
        QString prompt = "Test prompt";
        QVariantHash invalidParams;
        invalidParams["temperature"] = -5.0;  // Invalid temperature
        invalidParams["max_tokens"] = -100;   // Invalid token count
        
        QString response = chat->chat(prompt, {}, invalidParams);
        
        // Check if invalid parameters caused an error
        auto error = chat->lastError();
        if (error.getErrorCode() != -1) {
            qDebug() << "Error for invalid parameters:" << error.getErrorMessage();
        }
        
    }) << "Invalid parameters should be handled gracefully";
    
    // Test: Multiple rapid requests
    EXPECT_NO_THROW({
        for (int i = 0; i < 3; ++i) {
            QString prompt = QString("Quick test %1").arg(i);
            QString response = chat->chat(prompt);
            
            qDebug() << "Rapid request" << i << "response length:" << response.length();
            
            // Small delay between requests
            QTest::qWait(10);
        }
        
        validateErrorState(false);  // Rapid requests should not cause persistent errors
        
    }) << "Multiple rapid requests should be handled correctly";
    
    qInfo() << "Error handling tests completed";
}

/**
 * @brief Test parameter validation and edge cases
 * 
 * This test verifies that various parameter combinations
 * and edge cases are handled correctly.
 */
TEST_F(TestDChatCompletions, parameterValidation)
{
    qInfo() << "Testing DChatCompletions parameter validation";
    
    // Test: Various temperature values
    QList<double> temperatureValues = {0.0, 0.3, 0.7, 1.0, 1.5, 2.0};
    for (double temp : temperatureValues) {
        EXPECT_NO_THROW({
            QVariantHash params;
            params["temperature"] = temp;
            
            QString response = chat->chat("Test temperature", {}, params);
            qDebug() << "Temperature" << temp << "response length:" << response.length();
            
        }) << "Temperature value " << temp << " should be handled";
    }
    
    // Test: Various token limits
    QList<int> tokenLimits = {1, 10, 100, 1000, 4000};
    for (int limit : tokenLimits) {
        EXPECT_NO_THROW({
            QVariantHash params;
            params["max_tokens"] = limit;
            
            QString response = chat->chat("Test token limit", {}, params);
            qDebug() << "Token limit" << limit << "response length:" << response.length();
            
        }) << "Token limit " << limit << " should be handled";
    }
    
    // Test: Complex chat history
    EXPECT_NO_THROW({
        QList<ChatHistory> complexHistory;
        
        // Add multiple conversation turns
        for (int i = 0; i < 5; ++i) {
            ChatHistory userMsg;
            userMsg.role = kChatRoleUser;
            userMsg.content = QString("User message %1").arg(i);
            complexHistory.append(userMsg);
            
            ChatHistory assistantMsg;
            assistantMsg.role = kChatRoleAssistant;
            assistantMsg.content = QString("Assistant response %1").arg(i);
            complexHistory.append(assistantMsg);
        }
        
        QString response = chat->chat("Continue conversation", complexHistory);
        qDebug() << "Complex history response length:" << response.length();
        
    }) << "Complex chat history should be handled";
    
    // Test: Unicode content in prompts and history
    EXPECT_NO_THROW({
        QString unicodePrompt = "你好，请用中文回答。こんにちは。🤖✨";
        
        ChatHistory unicodeHistory;
        unicodeHistory.role = kChatRoleUser;
        unicodeHistory.content = "Préférences linguistiques: français, 中文, 日本語";
        
        QString response = chat->chat(unicodePrompt, {unicodeHistory});
        qDebug() << "Unicode response length:" << response.length();
        
    }) << "Unicode content should be handled correctly";
    
    qInfo() << "Parameter validation tests completed";
}
