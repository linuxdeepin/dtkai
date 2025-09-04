// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TEST_BASE_H
#define TEST_BASE_H

#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QTest>
#include <QDebug>

/**
 * @brief 通用测试基类
 * 
 * 提供基础的测试环境设置和工具方法
 * 专注于接口测试，暂不包含Mock功能
 */
class TestBase : public testing::Test
{
public:
    void SetUp() override
    {
        setupTestEnvironment();
    }
    
    void TearDown() override
    {
        cleanupTestEnvironment();
    }

protected:
    /**
     * @brief 等待异步操作完成
     * @param timeoutMs 超时时间（毫秒）
     */
    void waitForAsync(int timeoutMs = 5000)
    {
        QTest::qWait(timeoutMs);
    }
    
    /**
     * @brief 检查字符串是否非空
     */
    void expectNonEmptyString(const QString &str, const QString &description = "")
    {
        EXPECT_FALSE(str.isEmpty()) << description.toStdString();
        EXPECT_FALSE(str.isNull()) << description.toStdString();
    }
    
    /**
     * @brief 检查列表是否非空
     */
    template<typename T>
    void expectNonEmptyList(const QList<T> &list, const QString &description = "")
    {
        EXPECT_FALSE(list.isEmpty()) << description.toStdString();
    }

private:
    void setupTestEnvironment()
    {
        // 确保有Qt应用程序实例
        if (!QCoreApplication::instance()) {
            int argc = 0;
            char **argv = nullptr;
            app = new QCoreApplication(argc, argv);
        }
        
        qDebug() << "Test environment setup completed";
    }
    
    void cleanupTestEnvironment()
    {
        if (app && app != QCoreApplication::instance()) {
            delete app;
            app = nullptr;
        }
    }

private:
    QCoreApplication *app { nullptr };
};

#endif // TEST_BASE_H