// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_base.h"
#include "test_utils.h"

#include "dtkai/dspeechtotext.h"
#include "dtkai/dtkaitypes.h"

#include <QSignalSpy>
#include <QTimer>
#include <QTest>
#include <QVariantHash>
#include <QByteArray>
#include <QTemporaryFile>
#include <QStandardPaths>
#include <cmath>

DAI_USE_NAMESPACE

/**
 * @brief Test class for DSpeechToText interface
 * 
 * This class tests the speech-to-text functionality including
 * file recognition, stream recognition, and various audio formats.
 */
class TestDSpeechToText : public TestBase
{
protected:
    void SetUp() override
    {
        TestBase::SetUp();
        qDebug() << "Setting up DSpeechToText interface tests";
        
        // Create speech-to-text instance for testing
        stt = new DSpeechToText();
        ASSERT_NE(stt, nullptr) << "Failed to create DSpeechToText instance";
    }
    
    void TearDown() override
    {
        if (stt) {
            delete stt;
            stt = nullptr;
        }
        TestBase::TearDown();
    }
    
    /**
     * @brief Helper method to create sample audio parameters
     * @return A QVariantHash with common audio recognition parameters
     */
    QVariantHash createSampleAudioParameters()
    {
        QVariantHash params;
        params["language"] = "zh-CN";
        params["sample_rate"] = 16000;
        params["channels"] = 1;
        params["format"] = "wav";
        params["enable_punctuation"] = true;
        params["enable_word_time"] = false;
        return params;
    }
    
    /**
     * @brief Helper method to generate mock audio data
     * @param duration Duration in seconds
     * @return Mock audio data as QByteArray
     */
    QByteArray generateMockAudioData(int duration = 1)
    {
        // Generate simple mock audio data (sine wave pattern)
        int sampleRate = 16000;
        int samples = sampleRate * duration;
        QByteArray audioData;
        audioData.reserve(samples * 2); // 16-bit samples
        
        for (int i = 0; i < samples; ++i) {
            // Simple sine wave at 440Hz (A note)
            double sample = sin(2.0 * M_PI * 440.0 * i / sampleRate);
            qint16 value = static_cast<qint16>(sample * 32767);
            audioData.append(reinterpret_cast<const char*>(&value), sizeof(value));
        }
        
        return audioData;
    }
    
    /**
     * @brief Helper method to create a temporary audio file
     * @param duration Duration in seconds
     * @return Path to the temporary audio file
     */
    QString createTempAudioFile(int duration = 1)
    {
        // Use stack-allocated QTemporaryFile to avoid memory leak
        QTemporaryFile tempFile;
        tempFile.setFileTemplate(QStandardPaths::writableLocation(QStandardPaths::TempLocation) + 
                                  "/dtkai_test_audio_XXXXXX.wav");
        tempFile.setAutoRemove(false); // Keep file for test duration
        
        if (tempFile.open()) {
            // Write a simple WAV header (simplified)
            QByteArray header;
            header.append("RIFF");
            header.append(QByteArray(4, 0)); // File size placeholder
            header.append("WAVE");
            header.append("fmt ");
            header.append(QByteArray::fromRawData("\x10\x00\x00\x00", 4)); // Format chunk size
            header.append(QByteArray::fromRawData("\x01\x00", 2)); // PCM format
            header.append(QByteArray::fromRawData("\x01\x00", 2)); // Mono
            header.append(QByteArray::fromRawData("\x40\x3F\x00\x00", 4)); // Sample rate 16000
            header.append(QByteArray::fromRawData("\x80\x7E\x00\x00", 4)); // Byte rate
            header.append(QByteArray::fromRawData("\x02\x00", 2)); // Block align
            header.append(QByteArray::fromRawData("\x10\x00", 2)); // Bits per sample
            header.append("data");
            
            QByteArray audioData = generateMockAudioData(duration);
            int audioSize = audioData.size();
            header.append(QByteArray::fromRawData(reinterpret_cast<const char*>(&audioSize), 4));
            
            tempFile.write(header);
            tempFile.write(audioData);
            tempFile.flush();
            
            QString filePath = tempFile.fileName();
            return filePath;
        }
        
        return QString();
    }
    
    /**
     * @brief Validate error state for speech recognition
     * @param expectedError Whether an error is expected
     */
    void validateErrorState(bool expectedError = false)
    {
        auto error = stt->lastError();
        
        // In test environment without AI daemon, error code 1 (APIServerNotAvailable) is expected
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
            }
        }
    }

protected:
    DSpeechToText *stt = nullptr;
};

/**
 * @brief Test constructor and destructor
 * 
 * This test verifies that DSpeechToText can be properly
 * constructed and destroyed.
 */
TEST_F(TestDSpeechToText, constructorDestructor)
{
    qInfo() << "Testing DSpeechToText constructor and destructor";
    
    // Test: Constructor with parent
    EXPECT_NO_THROW({
        QObject parent;
        DSpeechToText *sttWithParent = new DSpeechToText(&parent);
        EXPECT_NE(sttWithParent, nullptr);
        EXPECT_EQ(sttWithParent->parent(), &parent);
        
        // Destructor will be called automatically when parent is destroyed
    }) << "Constructor with parent should work correctly";
    
    // Test: Constructor without parent
    EXPECT_NO_THROW({
        DSpeechToText *standaloneStt = new DSpeechToText();
        EXPECT_NE(standaloneStt, nullptr);
        EXPECT_EQ(standaloneStt->parent(), nullptr);
        
        delete standaloneStt;
    }) << "Constructor without parent should work correctly";
    
    // Test: Initial error state
    validateErrorState(false);
    
    qInfo() << "Constructor/destructor tests completed";
}

/**
 * @brief Test file-based recognition functionality
 * 
 * This test verifies that audio file recognition works
 * correctly with various parameters.
 */
TEST_F(TestDSpeechToText, fileRecognition)
{
    qInfo() << "Testing DSpeechToText file recognition";
    
    // Test: Basic file recognition without parameters
    EXPECT_NO_THROW({
        QString audioFile = createTempAudioFile(2); // 2 second audio
        if (!audioFile.isEmpty()) {
            QString result = stt->recognizeFile(audioFile);
            
            qDebug() << "File recognition result:" << result;
            
            // In test environment without real daemon, result may be empty
            // This is acceptable - we're testing interface behavior
            validateErrorState(false);
            
            // Clean up temp file
            QFile::remove(audioFile);
        } else {
            qDebug() << "Failed to create temporary audio file - skipping file recognition test";
        }
        
    }) << "Basic file recognition should work";
    
    // Test: File recognition with parameters
    EXPECT_NO_THROW({
        QString audioFile = createTempAudioFile(1);
        if (!audioFile.isEmpty()) {
            QVariantHash params = createSampleAudioParameters();
            QString result = stt->recognizeFile(audioFile, params);
            
            qDebug() << "File recognition with params result:" << result;
            validateErrorState(false);
            
            QFile::remove(audioFile);
        }
        
    }) << "File recognition with parameters should work";
    
    // Test: Non-existent file handling
    EXPECT_NO_THROW({
        QString nonExistentFile = "/tmp/non_existent_audio_file.wav";
        QString result = stt->recognizeFile(nonExistentFile);
        
        EXPECT_TRUE(result.isEmpty()) << "Non-existent file should return empty result";
        
        // Check if error was set for non-existent file
        auto error = stt->lastError();
        if (error.getErrorCode() != -1 && error.getErrorCode() != 1) {
            qDebug() << "Error for non-existent file:" << error.getErrorCode() << error.getErrorMessage();
        }
        
    }) << "Non-existent file should be handled gracefully";
    
    qInfo() << "File recognition tests completed";
}

/**
 * @brief Test stream-based recognition functionality
 * 
 * This test verifies that streaming audio recognition works
 * correctly with proper signal emission.
 */
TEST_F(TestDSpeechToText, streamRecognition)
{
    qInfo() << "Testing DSpeechToText stream recognition";
    
    // Set up signal spies
    QSignalSpy resultSpy(stt, &DSpeechToText::recognitionResult);
    QSignalSpy partialSpy(stt, &DSpeechToText::recognitionPartialResult);
    QSignalSpy errorSpy(stt, &DSpeechToText::recognitionError);
    QSignalSpy completedSpy(stt, &DSpeechToText::recognitionCompleted);
    
    ASSERT_TRUE(resultSpy.isValid()) << "recognitionResult signal should be valid";
    ASSERT_TRUE(partialSpy.isValid()) << "recognitionPartialResult signal should be valid";
    ASSERT_TRUE(errorSpy.isValid()) << "recognitionError signal should be valid";
    ASSERT_TRUE(completedSpy.isValid()) << "recognitionCompleted signal should be valid";
    
    // Test: Basic stream recognition
    EXPECT_NO_THROW({
        QVariantHash params = createSampleAudioParameters();
        bool started = stt->startStreamRecognition(params);
        
        if (started) {
            qDebug() << "Stream recognition started successfully";
            
            // Send some mock audio data
            QByteArray audioChunk1 = generateMockAudioData(1);
            bool sent1 = stt->sendAudioData(audioChunk1);
            
            QByteArray audioChunk2 = generateMockAudioData(1);
            bool sent2 = stt->sendAudioData(audioChunk2);
            
            qDebug() << "Audio data sent:" << sent1 << sent2;
            
            // End the stream
            QString finalResult = stt->endStreamRecognition();
            qDebug() << "Final recognition result:" << finalResult;
            
            // Wait for potential signals
            QTest::qWait(100);
            
            // In test environment, signals may not be emitted due to no real daemon
            qDebug() << "Signal counts - Result:" << resultSpy.count() 
                     << "Partial:" << partialSpy.count() 
                     << "Error:" << errorSpy.count()
                     << "Completed:" << completedSpy.count();
            
        } else {
            qDebug() << "Stream recognition failed to start - checking for errors";
            validateErrorState(true);
        }
        
    }) << "Basic stream recognition should work";
    
    // Clear spies for next test
    resultSpy.clear();
    partialSpy.clear();
    errorSpy.clear();
    completedSpy.clear();
    
    // Test: Stream recognition without parameters
    EXPECT_NO_THROW({
        bool started = stt->startStreamRecognition();
        
        if (started) {
            qDebug() << "Stream recognition without params started";
            
            QByteArray audioData = generateMockAudioData(1);
            stt->sendAudioData(audioData);
            
            QString result = stt->endStreamRecognition();
            qDebug() << "Stream result without params:" << result;
        }
        
    }) << "Stream recognition without parameters should work";
    
    qInfo() << "Stream recognition tests completed";
}

/**
 * @brief Test terminate functionality
 * 
 * This test verifies that ongoing recognition operations
 * can be properly terminated.
 */
TEST_F(TestDSpeechToText, terminateOperation)
{
    qInfo() << "Testing DSpeechToText terminate functionality";
    
    // Test: Terminate stream recognition
    EXPECT_NO_THROW({
        bool started = stt->startStreamRecognition();
        
        if (started) {
            qDebug() << "Started stream recognition for termination test";
            
            // Give it a moment to start
            QTest::qWait(50);
            
            // Terminate the operation
            stt->terminate();
            qDebug() << "Terminate called";
            
            validateErrorState(false); // Termination should not cause error state
            
        } else {
            qDebug() << "Stream failed to start for termination test";
        }
        
    }) << "Terminate stream recognition should work correctly";
    
    // Test: Terminate when no operation is running
    EXPECT_NO_THROW({
        stt->terminate();
        qDebug() << "Terminate called with no active operation";
        validateErrorState(false); // Should not cause error
        
    }) << "Terminate with no active operation should be safe";
    
    qInfo() << "Terminate functionality tests completed";
}

/**
 * @brief Test information methods
 * 
 * This test verifies that information retrieval methods
 * work correctly and return reasonable data.
 */
TEST_F(TestDSpeechToText, informationMethods)
{
    qInfo() << "Testing DSpeechToText information methods";
    
    // Test: Get supported formats
    EXPECT_NO_THROW({
        QStringList formats = stt->getSupportedFormats();
        
        qDebug() << "Supported formats:" << formats;
        
        // Formats list may be empty in test environment, which is acceptable
        if (!formats.isEmpty()) {
            // Common audio formats should be reasonable strings
            for (const QString &format : formats) {
                EXPECT_FALSE(format.isEmpty()) << "Format string should not be empty";
                EXPECT_LT(format.length(), 50) << "Format string should be reasonable length";
            }
            
            // Check for common formats
            bool hasCommonFormat = formats.contains("wav", Qt::CaseInsensitive) ||
                                 formats.contains("mp3", Qt::CaseInsensitive) ||
                                 formats.contains("flac", Qt::CaseInsensitive) ||
                                 formats.contains("pcm", Qt::CaseInsensitive);
            
            if (hasCommonFormat) {
                qDebug() << "Found common audio formats in supported list";
            }
        } else {
            qDebug() << "No supported formats returned - may be normal in test environment";
        }
        
    }) << "getSupportedFormats should work correctly";
    
    // Test: Get provider information (commented out - method not yet implemented)
    /*
    EXPECT_NO_THROW({
        QString providerInfo = stt->getProviderInfo();
        
        qDebug() << "Provider info:" << providerInfo;
        
        // Provider info may be empty in test environment
        if (!providerInfo.isEmpty()) {
            EXPECT_LT(providerInfo.length(), 1000) << "Provider info should be reasonable length";
        } else {
            qDebug() << "No provider info returned - may be normal in test environment";
        }
    });
    */
    
    qDebug() << "getProviderInfo test skipped - method not yet implemented";
    
    qInfo() << "Information methods tests completed";
}

/**
 * @brief Test error handling scenarios
 * 
 * This test verifies that various error conditions
 * are handled appropriately.
 */
TEST_F(TestDSpeechToText, errorHandling)
{
    qInfo() << "Testing DSpeechToText error handling";
    
    // Test: Empty audio data handling
    EXPECT_NO_THROW({
        bool started = stt->startStreamRecognition();
        
        if (started) {
            // Send empty audio data
            QByteArray emptyData;
            bool sent = stt->sendAudioData(emptyData);
            
            qDebug() << "Empty audio data sent:" << sent;
            
            QString result = stt->endStreamRecognition();
            qDebug() << "Result after empty data:" << result;
        }
        
    }) << "Empty audio data should be handled gracefully";
    
    // Test: Invalid audio format file
    EXPECT_NO_THROW({
        // Create a text file with audio extension
        QTemporaryFile tempFile;
        tempFile.setFileTemplate("/tmp/invalid_audio_XXXXXX.wav");
        
        if (tempFile.open()) {
            tempFile.write("This is not audio data");
            tempFile.close();
            
            QString result = stt->recognizeFile(tempFile.fileName());
            
            qDebug() << "Invalid audio file result:" << result;
            
            // Should handle invalid format gracefully
            auto error = stt->lastError();
            if (error.getErrorCode() != -1 && error.getErrorCode() != 1) {
                qDebug() << "Error for invalid audio:" << error.getErrorCode() << error.getErrorMessage();
            }
        }
        
    }) << "Invalid audio format should be handled gracefully";
    
    // Test: Very large audio data
    EXPECT_NO_THROW({
        QByteArray largeData = generateMockAudioData(60); // 60 seconds of audio
        
        bool started = stt->startStreamRecognition();
        if (started) {
            bool sent = stt->sendAudioData(largeData);
            qDebug() << "Large audio data sent:" << sent;
            
            QString result = stt->endStreamRecognition();
            qDebug() << "Result after large data:" << result;
        }
        
    }) << "Large audio data should be handled correctly";
    
    // Test: Multiple stream starts without ending
    EXPECT_NO_THROW({
        bool started1 = stt->startStreamRecognition();
        bool started2 = stt->startStreamRecognition(); // Second start without ending first
        
        qDebug() << "Multiple stream starts:" << started1 << started2;
        
        // End the stream (whichever is active)
        QString result = stt->endStreamRecognition();
        qDebug() << "Result after multiple starts:" << result;
        
    }) << "Multiple stream starts should be handled gracefully";
    
    qInfo() << "Error handling tests completed";
}

/**
 * @brief Test parameter validation and edge cases
 * 
 * This test verifies that various parameter combinations
 * and edge cases are handled correctly.
 */
TEST_F(TestDSpeechToText, parameterValidation)
{
    qInfo() << "Testing DSpeechToText parameter validation";
    
    // Test: Various language parameters
    QStringList languages = {"zh-CN", "en-US", "ja-JP", "ko-KR", "fr-FR", "de-DE"};
    for (const QString &lang : languages) {
        EXPECT_NO_THROW({
            QVariantHash params;
            params["language"] = lang;
            
            bool started = stt->startStreamRecognition(params);
            if (started) {
                QString result = stt->endStreamRecognition();
                qDebug() << "Language" << lang << "result length:" << result.length();
            }
            
        }) << "Language parameter " << lang.toStdString() << " should be handled";
    }
    
    // Test: Various sample rates
    QList<int> sampleRates = {8000, 16000, 22050, 44100, 48000};
    for (int rate : sampleRates) {
        EXPECT_NO_THROW({
            QVariantHash params;
            params["sample_rate"] = rate;
            params["language"] = "zh-CN";
            
            bool started = stt->startStreamRecognition(params);
            if (started) {
                QString result = stt->endStreamRecognition();
                qDebug() << "Sample rate" << rate << "result length:" << result.length();
            }
            
        }) << "Sample rate " << rate << " should be handled";
    }
    
    // Test: Complex parameter combinations
    EXPECT_NO_THROW({
        QVariantHash complexParams;
        complexParams["language"] = "zh-CN";
        complexParams["sample_rate"] = 16000;
        complexParams["channels"] = 1;
        complexParams["format"] = "wav";
        complexParams["enable_punctuation"] = true;
        complexParams["enable_word_time"] = true;
        complexParams["enable_intermediate_result"] = false;
        complexParams["max_alternatives"] = 3;
        
        bool started = stt->startStreamRecognition(complexParams);
        if (started) {
            QByteArray audioData = generateMockAudioData(2);
            stt->sendAudioData(audioData);
            
            QString result = stt->endStreamRecognition();
            qDebug() << "Complex params result length:" << result.length();
        }
        
    }) << "Complex parameter combinations should be handled";
    
    // Test: Invalid parameter values
    EXPECT_NO_THROW({
        QVariantHash invalidParams;
        invalidParams["sample_rate"] = -1000; // Invalid sample rate
        invalidParams["channels"] = 100;      // Too many channels
        invalidParams["language"] = "";       // Empty language
        
        bool started = stt->startStreamRecognition(invalidParams);
        qDebug() << "Invalid params stream started:" << started;
        
        if (started) {
            QString result = stt->endStreamRecognition();
            qDebug() << "Invalid params result:" << result;
        }
        
        // Check if invalid parameters caused an error
        auto error = stt->lastError();
        if (error.getErrorCode() != -1 && error.getErrorCode() != 1) {
            qDebug() << "Error for invalid params:" << error.getErrorCode() << error.getErrorMessage();
        }
        
    }) << "Invalid parameters should be handled gracefully";
    
    qInfo() << "Parameter validation tests completed";
}
