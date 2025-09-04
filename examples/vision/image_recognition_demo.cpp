// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

/**
 * @file image_recognition_demo.cpp
 * @brief Image Recognition Demo for testing dtkai DImageRecognition interface
 * 
 * This demo tests the image recognition functionality using the specified test image URL.
 * It demonstrates how to use the DImageRecognition class for recognizing images.
 */
#include <DAIError>
#include <DImageRecognition>

#include <QCoreApplication>
#include <QDebug>
#include <QTimer>
#include <QThread>

DAI_USE_NAMESPACE

class ImageRecognitionDemo : public QObject
{
    Q_OBJECT

public:
    ImageRecognitionDemo(QObject *parent = nullptr) : QObject(parent)
    {
        imageRecognition.reset(new DImageRecognition(this));
    }

    ~ImageRecognitionDemo()
    {
        if (imageRecognition) {
            imageRecognition->terminate();
        }
    }

public Q_SLOTS:
    void runTests()
    {
        qInfo() << "=== 图像识别 Demo 开始 ===";
        qInfo() << "";
        
        // Test connection and basic information
        testConnectionAndInfo();
        
        // Test image recognition with URL
        testImageRecognitionWithUrl();
        
        // Schedule app exit
        QTimer::singleShot(5000, this, [this]() {
            qInfo() << "";
            qInfo() << "=== Demo 完成 ===";
            qApp->quit();
        });
    }

private:
    void testConnectionAndInfo()
    {
        qInfo() << "🔗 测试连接并获取信息...";
        
        // Test supported image formats
        QStringList formats = imageRecognition->getSupportedImageFormats();
        auto error = imageRecognition->lastError();
        
        if (error.getErrorCode() != -1) {
            qWarning() << "✗ 获取支持格式失败:" << error.getErrorCode() << error.getErrorMessage();
        } else {
            qInfo() << "✓ 支持的图像格式:" << formats;
        }
        
        // Test max image size
        int maxSize = imageRecognition->getMaxImageSize();
        error = imageRecognition->lastError();
        
        if (error.getErrorCode() != -1) {
            qWarning() << "✗ 获取最大图像尺寸失败:" << error.getErrorCode() << error.getErrorMessage();
        } else {
            qInfo() << "✓ 最大图像尺寸:" << maxSize << "bytes (" << (maxSize / 1024 / 1024) << "MB)";
        }
        
        qInfo() << "";
    }

    void testImageRecognitionWithUrl()
    {
        qInfo() << "🖼️  使用 URL 测试图像识别...";
        
        // Test image URL provided by user
        QString imageUrl = "https://ark-project.tos-cn-beijing.ivolces.com/images/view.jpeg";
        QString prompt = "请详细描述这张图片的内容，包括看到了什么物体、场景、人物或文字等。";
        
        qInfo() << "🖼️  图像 URL:" << imageUrl;
        qInfo() << "💭 提示词:" << prompt;
        qInfo() << "🔄 处理中...";
        
        QString result = imageRecognition->recognizeImageUrl(imageUrl, prompt);
        auto error = imageRecognition->lastError();
        
        if (error.getErrorCode() != -1) {
            qWarning() << "✗ 图像识别失败:" << error.getErrorCode() << error.getErrorMessage();
            
            // Print detailed error information
            if (error.getErrorCode() == static_cast<int>(AIErrorCode::APIServerNotAvailable)) {
                qWarning() << "   API 服务器不可用。请确保 deepin-ai-daemon 正在运行。";
            } else if (error.getErrorCode() == static_cast<int>(AIErrorCode::InvalidParameter)) {
                qWarning() << "   提供的参数无效。";
            } else {
                qWarning() << "   发生未知错误。";
            }
        } else if (result.isEmpty()) {
            qWarning() << "✗ 返回空结果";
        } else {
            qInfo() << "✅ 图像识别成功！";
            qInfo() << "📝 识别结果:";
            qInfo() << result;
        }
        
        qInfo() << "";
    }

private:
    QScopedPointer<DImageRecognition> imageRecognition;
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    // Set application information
    app.setApplicationName("Image Recognition Demo");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("UnionTech Software Technology Co., Ltd.");
    
    qInfo() << "图像识别 Demo - 版本 1.0.0";
    qInfo() << "测试 dtkai DImageRecognition 接口";
    qInfo() << "组织: UnionTech Software Technology Co., Ltd.";
    qInfo() << "";
    
    // Create and run demo
    ImageRecognitionDemo demo;
    
    // Start tests after a short delay
    QTimer::singleShot(100, &demo, &ImageRecognitionDemo::runTests);
    
    return app.exec();
}

#include "image_recognition_demo.moc"
