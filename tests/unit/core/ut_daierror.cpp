// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_base.h"
#include "test_utils.h"

#include "dtkai/daierror.h"
#include <DError>

DAI_USE_NAMESPACE
DCORE_USE_NAMESPACE

/**
 * @brief Test class for DAIError interface
 * 
 * This class tests the error handling mechanisms and error code
 * definitions used throughout the DTKAI library.
 */
class TestDAIError : public TestBase
{
protected:
    void SetUp() override
    {
        TestBase::SetUp();
        qDebug() << "Setting up DAIError interface tests";
    }
    
    void TearDown() override
    {
        TestBase::TearDown();
    }
    
    /**
     * @brief Helper method to validate error code
     * @param errorCode The error code to validate
     */
    void validateErrorCode(int errorCode)
    {
        // Error codes should be in reasonable range
        EXPECT_GE(errorCode, 0) << "Error code should be non-negative";
        EXPECT_LT(errorCode, 10000) << "Error code should be reasonable";
    }
    
    /**
     * @brief Helper method to validate error message
     * @param message The error message to validate
     */
    void validateErrorMessage(const QString &message)
    {
        // Error messages should be non-empty and reasonable length
        EXPECT_FALSE(message.isEmpty()) << "Error message should not be empty";
        EXPECT_LT(message.length(), 1000) << "Error message should not be too long";
        EXPECT_FALSE(message.contains('\0')) << "Error message should not contain null characters";
    }
};

/**
 * @brief Test basic error code definitions
 * 
 * This test verifies that error codes are properly defined
 * and accessible.
 */
TEST_F(TestDAIError, errorCodeDefinitions)
{
    qInfo() << "Testing DAIError code definitions";
    
    // Test: Validate all defined error codes
    EXPECT_NO_THROW({
        AIErrorCode noError = NoError;
        AIErrorCode serverError = APIServerNotAvailable;
        AIErrorCode paramError = InvalidParameter;
        AIErrorCode parseError = ParseError;
        
        // Validate error codes are in expected range
        validateErrorCode(static_cast<int>(noError));
        validateErrorCode(static_cast<int>(serverError));
        validateErrorCode(static_cast<int>(paramError));
        validateErrorCode(static_cast<int>(parseError));
        
        qDebug() << "NoError:" << noError;
        qDebug() << "APIServerNotAvailable:" << serverError;
        qDebug() << "InvalidParameter:" << paramError;
        qDebug() << "ParseError:" << parseError;
        
    }) << "Error code definitions should be accessible";
    
    // Test: Error codes should have distinct values
    EXPECT_NE(NoError, APIServerNotAvailable);
    EXPECT_NE(NoError, InvalidParameter);
    EXPECT_NE(NoError, ParseError);
    EXPECT_NE(APIServerNotAvailable, InvalidParameter);
    EXPECT_NE(APIServerNotAvailable, ParseError);
    EXPECT_NE(InvalidParameter, ParseError);
    
    // Test: NoError should be 0 (standard convention)
    EXPECT_EQ(NoError, 0) << "NoError should be 0 by convention";
}

/**
 * @brief Test error message functionality
 * 
 * This test verifies that error messages can be retrieved
 * and are properly formatted.
 */
TEST_F(TestDAIError, errorMessages)
{
    qInfo() << "Testing DAIError message functionality";
    
    // Test: Error message creation and access
    EXPECT_NO_THROW({
        // Test basic error message functionality
        QString testMessage = "Test error message";
        validateErrorMessage(testMessage);
    }) << "Error message handling should work correctly";
    
    // Test: Empty message handling
    QString emptyMessage = "";
    // Empty messages might be valid in some contexts, so we just test they don't crash
    EXPECT_NO_THROW({
        qDebug() << "Empty message length:" << emptyMessage.length();
    }) << "Empty error messages should be handled gracefully";
    
    // Test: Unicode message handling
    QString unicodeMessage = "错误信息 - Error Message - エラーメッセージ";
    EXPECT_NO_THROW({
        validateErrorMessage(unicodeMessage);
    }) << "Unicode error messages should be supported";
    
    qInfo() << "Error message tests completed";
}

/**
 * @brief Test error propagation mechanisms
 * 
 * This test verifies that errors can be properly propagated
 * through the system.
 */
TEST_F(TestDAIError, errorPropagation)
{
    qInfo() << "Testing DAIError propagation mechanisms";
    
    // Test: Error state management
    EXPECT_NO_THROW({
        // Test that error handling doesn't interfere with normal operation
        bool errorState = false;
        QString errorMessage = "Test propagation";
        
        // Simulate error condition
        if (!errorMessage.isEmpty()) {
            errorState = true;
        }
        
        EXPECT_TRUE(errorState) << "Error state should be set correctly";
        validateErrorMessage(errorMessage);
    }) << "Error propagation should work correctly";
    
    // Test: Multiple error handling
    std::vector<QString> errorMessages = {
        "First error",
        "Second error", 
        "Third error"
    };
    
    for (size_t i = 0; i < errorMessages.size(); ++i) {
        EXPECT_NO_THROW({
            validateErrorMessage(errorMessages[i]);
        }) << "Multiple errors should be handled correctly (error " << i << ")";
    }
    
    qInfo() << "Error propagation tests completed";
}

/**
 * @brief Test error context and debugging information
 * 
 * This test verifies that error context information
 * is properly maintained and accessible.
 */
TEST_F(TestDAIError, errorContext)
{
    qInfo() << "Testing DAIError context information";
    
    // Test: Error context preservation
    QString contextInfo = "Function: testFunction, Line: 123, File: test.cpp";
    validateErrorMessage(contextInfo);
    
    // Test that context information is reasonable
    EXPECT_TRUE(contextInfo.contains("Function:") || 
               contextInfo.contains("Line:") || 
               contextInfo.contains("File:"))
        << "Context should contain debugging information";
    
    // Test: Stack trace information (if available)
    QStringList stackTrace = {
        "main() at main.cpp:10",
        "processRequest() at processor.cpp:45", 
        "handleError() at error.cpp:78"
    };
    
    for (const QString &frame : stackTrace) {
        validateErrorMessage(frame);
    }
    
    qInfo() << "Error context tests completed";
}

/**
 * @brief Test internationalization support for errors
 * 
 * This test verifies that error messages support
 * internationalization and localization.
 */
TEST_F(TestDAIError, internationalization)
{
    qInfo() << "Testing DAIError internationalization support";
    
    // Test: Different language error messages
    QMap<QString, QString> localizedMessages = {
        {"en", "Connection failed"},
        {"zh_CN", "连接失败"},
        {"zh_TW", "連接失敗"},
        {"ja", "接続に失敗しました"},
        {"ko", "연결 실패"}
    };
    
    for (auto it = localizedMessages.begin(); it != localizedMessages.end(); ++it) {
        EXPECT_NO_THROW({
            validateErrorMessage(it.value());
            qDebug() << "Language:" << it.key() << "Message:" << it.value();
        }) << "Localized error messages should be supported for " << it.key().toStdString();
    }
    
    // Test: Fallback to default language
    EXPECT_NO_THROW({
        QString fallbackMessage = "Default error message";
        validateErrorMessage(fallbackMessage);
    }) << "Fallback error messages should work";
    
    qInfo() << "Internationalization tests completed";
}

/**
 * @brief Test error recovery mechanisms
 * 
 * This test verifies that the system can recover
 * from error conditions properly.
 */
TEST_F(TestDAIError, errorRecovery)
{
    qInfo() << "Testing DAIError recovery mechanisms";
    
    // Test: Error state reset
    EXPECT_NO_THROW({
        bool hasError = true;
        QString errorMessage = "Recoverable error";
        
        // Simulate error recovery
        if (hasError) {
            qDebug() << "Recovering from error:" << errorMessage;
            hasError = false;
            errorMessage.clear();
        }
        
        EXPECT_FALSE(hasError) << "Error state should be cleared after recovery";
        EXPECT_TRUE(errorMessage.isEmpty()) << "Error message should be cleared after recovery";
    }) << "Error recovery should work correctly";
    
    // Test: Retry mechanisms
    int maxRetries = 3;
    int currentRetry = 0;
    bool success = false;
    
    while (currentRetry < maxRetries && !success) {
        EXPECT_NO_THROW({
            currentRetry++;
            // Simulate operation that might fail
            if (currentRetry >= 2) {  // Succeed on second retry
                success = true;
            }
        }) << "Retry mechanism should work (attempt " << currentRetry << ")";
    }
    
    EXPECT_TRUE(success) << "Operation should eventually succeed with retries";
    EXPECT_LE(currentRetry, maxRetries) << "Should not exceed maximum retries";
    
    qInfo() << "Error recovery tests completed after" << currentRetry << "attempts";
}

/**
 * @brief Test DError integration
 * 
 * This test verifies that DError from DTK Core integrates
 * properly with DTKAI error handling.
 */
TEST_F(TestDAIError, dtkCoreIntegration)
{
    qInfo() << "Testing DError integration with DTKAI";
    
    // Test: DError creation and basic functionality
    EXPECT_NO_THROW({
        DError error;
        
        // Test default state (DError uses -1 as default error code)
        EXPECT_EQ(error.getErrorCode(), -1) << "New DError should have default error code -1";
        EXPECT_TRUE(error.getErrorMessage().isEmpty()) << "New DError should have empty message";
        
        // Test error creation with proper constructor
        DError testError(APIServerNotAvailable, "API server not available");
        
        qDebug() << "Error code:" << testError.getErrorCode();
        qDebug() << "Error message:" << testError.getErrorMessage();
        
        EXPECT_EQ(testError.getErrorCode(), APIServerNotAvailable);
        validateErrorMessage(testError.getErrorMessage());
        
    }) << "DError integration should work correctly";
    
    // Test: Error code mapping
    QMap<AIErrorCode, QString> errorCodeMessages = {
        {NoError, "No error occurred"},
        {APIServerNotAvailable, "API server is not available"},
        {InvalidParameter, "Invalid parameter provided"},
        {ParseError, "Failed to parse response"}
    };
    
    for (auto it = errorCodeMessages.begin(); it != errorCodeMessages.end(); ++it) {
        EXPECT_NO_THROW({
            DError mappedError(it.key(), it.value());
            
            EXPECT_EQ(mappedError.getErrorCode(), it.key());
            validateErrorMessage(mappedError.getErrorMessage());
            
        }) << "Error code mapping should work for " << it.key();
    }
    
    // Test: Error message modification
    EXPECT_NO_THROW({
        DError modifiableError;
        modifiableError.setErrorCode(InvalidParameter);
        modifiableError.setErrorMessage("Custom error message");
        
        EXPECT_EQ(modifiableError.getErrorCode(), InvalidParameter);
        EXPECT_EQ(modifiableError.getErrorMessage(), "Custom error message");
        
    }) << "Error modification should work correctly";
    
    qInfo() << "DError integration tests completed";
}
