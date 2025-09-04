// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QCoreApplication>
#include <QLoggingCategory>
#include <gtest/gtest.h>

#ifdef QT_DEBUG
#include <sanitizer/asan_interface.h>
#endif

int main(int argc, char **argv)
{
    // 设置环境变量
    qputenv("QT_LOGGING_RULES", "*.debug=true;*.info=true");
    
    // 初始化Qt应用程序
    QCoreApplication app(argc, argv);
    app.setApplicationName("DTKAI Unit Tests");
    app.setApplicationVersion("1.0.0");
    
    qInfo() << "Starting DTKAI Interface Tests...";
    qInfo() << "Focus: Testing public interfaces in dtkai/include";
    qInfo() << "Strategy: Interface-first, Mock-later approach";
    
    // 初始化Google Test
    ::testing::InitGoogleTest(&argc, argv);
    
    // 运行测试
    int result = RUN_ALL_TESTS();
    
    #ifdef QT_DEBUG
    // 设置AddressSanitizer报告路径
    __sanitizer_set_report_path("asan_dtkai.log");
    #endif
    
    qInfo() << "DTKAI Interface Tests completed with result:" << result;
    
    return result;
}