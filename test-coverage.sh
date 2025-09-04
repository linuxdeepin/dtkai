#!/bin/bash
# SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later

set -e

echo "========================================"
echo "DTKAI Interface Test with Coverage"
echo "========================================"

# 检查必要工具
check_tool() {
    if ! command -v $1 &> /dev/null; then
        echo "❌ $1 is not installed"
        exit 1
    fi
}

echo "Checking required tools..."
check_tool cmake
check_tool lcov
check_tool genhtml
echo "✓ All required tools are available"

# 设置环境变量
export QT_LOGGING_RULES="*.debug=true;*.info=true"

# 清理之前的构建
#echo "Cleaning previous build..."
#rm -rf build

# 配置构建
echo "Configuring build environment..."
cmake -B build -DCMAKE_BUILD_TYPE=Debug \
    -DBUILD_TESTING=ON \
    -DCMAKE_CXX_FLAGS="--coverage" \
    -DCMAKE_EXE_LINKER_FLAGS="--coverage"

if [ $? -ne 0 ]; then
    echo "❌ CMake configuration failed!"
    exit 1
fi
echo "✓ Build configured successfully"

# 构建测试
echo "Building test executable..."
cmake --build build --target ut-dtkai -- -j $(nproc)

if [ $? -ne 0 ]; then
    echo "❌ Build failed!"
    exit 1
fi
echo "✓ Build completed successfully"

# 运行测试
echo "Running interface tests..."
echo "----------------------------------------"
cd build && ctest --output-on-failure --verbose

if [ $? -ne 0 ]; then
    echo "❌ Tests failed!"
    exit 1
fi
cd ..
echo "----------------------------------------"
echo "✓ All tests passed!"

# 生成覆盖率报告
echo "Generating coverage report..."

# 捕获覆盖率数据（与dtkio保持一致）
lcov -d build/ -c -o build/coverage_all.info
lcov --remove build/coverage_all.info "*/tests/*" "*/usr/include*" "*/moc*.cpp" --output-file build/coverage.info

# 生成HTML报告
genhtml build/coverage.info --output-directory build/coverage_html \
    --title "DTKAI Interface Test Coverage" \
    --show-details --legend

if [ $? -eq 0 ]; then
    echo "✓ Coverage report generated successfully!"
    echo "📊 Coverage report: build/coverage_html/index.html"
    
    # 显示覆盖率摘要
    echo ""
    echo "Coverage Summary:"
    echo "=================="
    lcov --summary build/coverage.info
else
    echo "⚠️  Coverage report generation completed with warnings"
fi

echo ""
echo "=========================================="
echo "DTKAI Interface Test Summary"
echo "=========================================="
echo "✓ Build: Success"
echo "✓ Tests: All Passed" 
echo "✓ Coverage: Report Generated"
echo "🎯 Focus: Interface Testing"
echo "📁 Report: build/coverage_html/index.html"
echo "=========================================="