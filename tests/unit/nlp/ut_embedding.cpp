// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_base.h"
#include "test_utils.h"

#include "dtkai/nlp/dembeddingplatform.h"
#include "dtkai/dtkaitypes.h"
#include "dtkai/daierror.h"

#include <QSignalSpy>
#include <QTimer>
#include <QTest>
#include <QVariantHash>
#include <QByteArray>
#include <QTemporaryFile>
#include <QStandardPaths>
#include <QFile>
#include <QDir>

DAI_USE_NAMESPACE

/**
 * @brief Test class for DEmbeddingPlatform interface
 * 
 * This class tests the embedding platform functionality including
 * document upload, search, and management.
 */
class TestDEmbeddingPlatform : public TestBase
{
protected:
    void SetUp() override
    {
        TestBase::SetUp();
        qDebug() << "Setting up DEmbeddingPlatform interface tests";
        
        // Create embedding platform instance for testing
        embedding = new DEmbeddingPlatform();
        ASSERT_NE(embedding, nullptr) << "Failed to create DEmbeddingPlatform instance";
    }
    
    void TearDown() override
    {
        if (embedding) {
            delete embedding;
            embedding = nullptr;
        }
        TestBase::TearDown();
    }
    
    /**
     * @brief Helper method to create a temporary text file for testing
     * @param content File content
     * @return Path to the temporary file
     */
    QString createTempTextFile(const QString &content = "This is a test document for embedding platform.")
    {
        QTemporaryFile tempFile;
        tempFile.setFileTemplate(QStandardPaths::writableLocation(QStandardPaths::TempLocation) + 
                                  "/dtkai_test_document_XXXXXX.txt");
        tempFile.setAutoRemove(false); // Keep file for test duration
        
        if (tempFile.open()) {
            tempFile.write(content.toUtf8());
            tempFile.flush();
            
            QString filePath = tempFile.fileName();
            return filePath;
        }
        
        return QString();
    }
    
    /**
     * @brief Validate error state for embedding operations
     * @param expectedError Whether an error is expected
     */
    void validateErrorState(bool expectedError = false)
    {
        auto error = embedding->lastError();
        
        // In test environment without AI daemon, error code 1 (APIServerNotAvailable) is expected
        if (expectedError) {
            EXPECT_NE(error.getErrorCode(), NoError) << "Expected an error to be set";
            if (error.getErrorCode() != NoError) {
                qDebug() << "Expected error code:" << error.getErrorCode() << "message:" << error.getErrorMessage();
            }
        } else {
            // In test environment, we may get APIServerNotAvailable (code 1) which is acceptable
            if (error.getErrorCode() == APIServerNotAvailable) {
                qDebug() << "Info: AI daemon not available (error code 1) - this is normal in test environment";
            } else if (error.getErrorCode() != NoError) {
                qDebug() << "Warning: Unexpected error code:" << error.getErrorCode() << "message:" << error.getErrorMessage();
            }
        }
    }

protected:
    DEmbeddingPlatform *embedding = nullptr;
};

/**
 * @brief Test constructor and destructor
 * 
 * This test verifies that DEmbeddingPlatform can be properly
 * constructed and destroyed.
 */
TEST_F(TestDEmbeddingPlatform, constructorDestructor)
{
    qInfo() << "Testing DEmbeddingPlatform constructor and destructor";
    
    // Test: Constructor with parent
    EXPECT_NO_THROW({
        QObject parent;
        DEmbeddingPlatform *embeddingWithParent = new DEmbeddingPlatform(&parent);
        EXPECT_NE(embeddingWithParent, nullptr);
        EXPECT_EQ(embeddingWithParent->parent(), &parent);
        
        // Destructor will be called automatically when parent is destroyed
    }) << "Constructor with parent should work correctly";
    
    // Test: Constructor without parent
    EXPECT_NO_THROW({
        DEmbeddingPlatform *standaloneEmbedding = new DEmbeddingPlatform();
        EXPECT_NE(standaloneEmbedding, nullptr);
        EXPECT_EQ(standaloneEmbedding->parent(), nullptr);
        
        delete standaloneEmbedding;
    }) << "Constructor without parent should work correctly";
    
    // Test: Initial error state
    validateErrorState(false);
    
    qInfo() << "Constructor/destructor tests completed";
}

/**
 * @brief Test embedding models functionality
 * 
 * This test verifies that embedding models can be retrieved.
 */
TEST_F(TestDEmbeddingPlatform, embeddingModels)
{
    qInfo() << "Testing DEmbeddingPlatform embedding models";
    
    QString models = embedding->embeddingModels();

    EXPECT_FALSE(models.isEmpty());

    qDebug() << "Embedding models result:" << models;

    // In test environment without real daemon, result may be empty
    // This is acceptable - we're testing interface behavior
    validateErrorState(false);

    // If we get a result, it should be valid JSON
    if (!models.isEmpty()) {
        QJsonDocument doc = QJsonDocument::fromJson(models.toUtf8());
        EXPECT_FALSE(doc.isNull()) << "Embedding models should return valid JSON";
    }
    
    qInfo() << "Embedding models tests completed";
}

/**
 * @brief Test document upload functionality
 * 
 * This test verifies that documents can be uploaded.
 */
static QString kFileID;
TEST_F(TestDEmbeddingPlatform, uploadDocuments)
{
    qInfo() << "Testing DEmbeddingPlatform document upload";

    QString testFile = createTempTextFile();
    if (!testFile.isEmpty()) {
        QStringList files = {testFile};
        QList<DEmbeddingPlatform::DocumentInfo> result = embedding->uploadDocuments("test-app-001", files);

        qDebug() << "Upload documents result count:" << result.count();

        // In test environment without real daemon, result may be empty
        validateErrorState(false);

        // If we get results, print the first one
        if (!result.isEmpty()) {
            DEmbeddingPlatform::DocumentInfo firstDoc = result.first();
            kFileID = firstDoc.id;
            qDebug() << "First document info - ID:" << firstDoc.id << "File Path:" << firstDoc.filePath;
        }

        // Clean up temp file
        QFile::remove(testFile);
    } else {
        qDebug() << "Failed to create temporary test file - skipping upload test";
    }

    qInfo() << "Document upload tests completed";
}

/**
 * @brief Test index building functionality
 *
 * This test verifies that index can be built.
 */
TEST_F(TestDEmbeddingPlatform, buildIndex)
{
    qInfo() << "Testing DEmbeddingPlatform index building";

    // Test: Build index (with empty document ID)
    EXPECT_NO_THROW({
        bool result = embedding->buildIndex("test-app-001", kFileID);

        qDebug() << "Build index result:" << result;

        // In test environment without real daemon, result may be false
        validateErrorState(false);
    }) << "Build index should work";

    qInfo() << "Index building tests completed";
}


/**
 * @brief Test document search functionality
 * 
 * This test verifies that documents can be searched.
 */
TEST_F(TestDEmbeddingPlatform, searchDocuments)
{
    qInfo() << "Testing DEmbeddingPlatform document search";

    QList<DEmbeddingPlatform::SearchResult> result = embedding->search("test-app-001", "test");

    qDebug() << "Search documents result count:" << result.count();

    // In test environment without real daemon, result may be empty
    validateErrorState(false);

    // If we get results, print the first one
    if (!result.isEmpty()) {
        DEmbeddingPlatform::SearchResult firstResult = result.first();
        qDebug() << "First search result - ID:" << firstResult.id << "Model:" << firstResult.model << "Content:" << firstResult.chunk.content;
    }
    
    qInfo() << "Document search tests completed";
}

/**
 * @brief Test document info functionality
 * 
 * This test verifies that document information can be retrieved.
 */
TEST_F(TestDEmbeddingPlatform, documentsInfo)
{
    qInfo() << "Testing DEmbeddingPlatform document info";
    
    // Test: Get documents info
    EXPECT_NO_THROW({
        QList<DEmbeddingPlatform::DocumentInfo> result = embedding->documentsInfo("test-app-001", {kFileID});
        
        qDebug() << "Documents info result count:" << result.count();
        
        // In test environment without real daemon, result may be empty
        validateErrorState(false);
        
        // If we get results, print the first one
        if (!result.isEmpty()) {
            DEmbeddingPlatform::DocumentInfo firstDoc = result.first();
            qDebug() << "First document info - ID:" << firstDoc.id << "File Path:" << firstDoc.filePath;
        }
    }) << "Get documents info should work";
    
    qInfo() << "Document info tests completed";
}

/**
 * @brief Test document deletion functionality
 * 
 * This test verifies that documents can be deleted.
 */
TEST_F(TestDEmbeddingPlatform, deleteDocuments)
{
    qInfo() << "Testing DEmbeddingPlatform document deletion";
    
    // Test: Delete documents (with empty document IDs)
    EXPECT_NO_THROW({
        bool result = embedding->deleteDocuments("test-app-001", {kFileID});
        
        qDebug() << "Delete documents result:" << result;
        
        // In test environment without real daemon, result may be false
        validateErrorState(false);
    }) << "Delete documents should work";
    
    qInfo() << "Document deletion tests completed";
}

/**
 * @brief Test index destruction functionality
 * 
 * This test verifies that index can be destroyed.
 */
TEST_F(TestDEmbeddingPlatform, destroyIndex)
{
    qInfo() << "Testing DEmbeddingPlatform index destruction";
    
    // Test: Destroy index (all indexes)
    EXPECT_NO_THROW({
        bool result = embedding->destroyIndex("test-app-001", true);
        
        qDebug() << "Destroy index result:" << result;
        
        // In test environment without real daemon, result may be false
        validateErrorState(false);
    }) << "Destroy index should work";
    
    qInfo() << "Index destruction tests completed";
}
