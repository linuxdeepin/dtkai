// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#include <QString>
#include <QByteArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QDebug>

/**
 * @brief 测试工具类
 * 
 * 提供测试数据加载、路径管理等实用功能
 */
class TestUtils
{
public:
    /**
     * @brief 获取测试资源文件路径
     * @param relativePath 相对路径
     * @return 完整的资源文件路径
     */
    static QString getResourcePath(const QString &relativePath)
    {
        // 测试资源目录
        QString resourceDir = QDir::currentPath() + "/tests/resources/";
        return QDir::cleanPath(resourceDir + relativePath);
    }
    
    /**
     * @brief 加载测试文件内容
     * @param filename 文件名（相对于resources目录）
     * @return 文件内容
     */
    static QByteArray loadTestFile(const QString &filename)
    {
        QString filePath = getResourcePath(filename);
        QFile file(filePath);
        
        if (!file.open(QIODevice::ReadOnly)) {
            qWarning() << "Failed to open test file:" << filePath;
            return QByteArray();
        }
        
        return file.readAll();
    }
    
    /**
     * @brief 加载测试JSON文件
     * @param filename JSON文件名
     * @return JSON对象
     */
    static QJsonObject loadTestJson(const QString &filename)
    {
        QByteArray data = loadTestFile(filename);
        if (data.isEmpty()) {
            return QJsonObject();
        }
        
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(data, &error);
        
        if (error.error != QJsonParseError::NoError) {
            qWarning() << "Failed to parse JSON file:" << filename << "Error:" << error.errorString();
            return QJsonObject();
        }
        
        return doc.object();
    }
    
    /**
     * @brief 生成测试用的音频数据（模拟）
     * @param sampleRate 采样率
     * @param duration 持续时间（秒）
     * @return 模拟的音频数据
     */
    static QByteArray generateAudioData(int sampleRate = 16000, int duration = 1)
    {
        // 简单生成一些模拟音频数据
        int dataSize = sampleRate * duration * 2; // 16-bit samples
        QByteArray audioData;
        audioData.reserve(dataSize);
        
        for (int i = 0; i < dataSize; ++i) {
            audioData.append(static_cast<char>(i % 256));
        }
        
        return audioData;
    }
    
    /**
     * @brief 生成测试用的图像数据（模拟）
     * @param width 宽度
     * @param height 高度
     * @return 模拟的图像数据
     */
    static QByteArray generateImageData(int width = 100, int height = 100)
    {
        // 简单生成一些模拟图像数据
        int dataSize = width * height * 3; // RGB
        QByteArray imageData;
        imageData.reserve(dataSize);
        
        for (int i = 0; i < dataSize; ++i) {
            imageData.append(static_cast<char>((i * 37) % 256));
        }
        
        return imageData;
    }
    
    /**
     * @brief 验证音频数据格式
     * @param data 音频数据
     * @return 是否为有效格式
     */
    static bool isValidAudioFormat(const QByteArray &data)
    {
        // 简单验证：非空且有合理的大小
        return !data.isEmpty() && data.size() > 100;
    }
    
    /**
     * @brief 验证图像数据格式
     * @param data 图像数据
     * @return 是否为有效格式
     */
    static bool isValidImageFormat(const QByteArray &data)
    {
        // 简单验证：非空且有合理的大小
        return !data.isEmpty() && data.size() > 100;
    }
    
    /**
     * @brief 创建临时测试文件
     * @param content 文件内容
     * @param suffix 文件后缀
     * @return 临时文件路径
     */
    static QString createTempFile(const QByteArray &content, const QString &suffix = ".tmp")
    {
        QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
        QString fileName = QString("dtkai_test_%1%2").arg(QDateTime::currentMSecsSinceEpoch()).arg(suffix);
        QString filePath = QDir(tempDir).absoluteFilePath(fileName);
        
        QFile file(filePath);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(content);
            file.close();
            return filePath;
        }
        
        return QString();
    }
};

#endif // TEST_UTILS_H
