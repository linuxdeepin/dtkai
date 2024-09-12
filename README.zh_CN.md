# Development Tool Kit AI

基于Qt风格的 Deepin AI 开发库。

## 依赖

### 编译依赖

-  cmake,
-  pkg-config,
-  qhelpgenerator-qt5 | qttools5-dev-tools,
-  qtbase5-dev,
-  qttools5-dev,
-  libdtkcore-dev

## 构建安装

### 从源码构建

1. 确保已经安装了所有的编译依赖.


   ```bash
   git clone https://github.com/linuxdeepin/dtkai
   cd dtkai
   sudo apt build-dep ./
   ```

2. 构建


   ```bash
   cmake -B build -DCMAKE_INSTALL_PREFIX=/usr
   cmake --build build
   ```

### 安装

```bash
sudo cmake --build build --target install
```

### 运行时依赖

- deepin-ai-daemon (>= 2.0.0)

## 帮助

任何使用问题都可以通过以下方式寻求帮助:

* [Telegram group](https://t.me/deepin)
* [Matrix](https://matrix.to/#/#deepin-community:matrix.org)
* [IRC (libera.chat)](https://web.libera.chat/#deepin-community)
* [Forum](https://bbs.deepin.org)
* [WiKi](https://wiki.deepin.org/)

## 参与贡献

我们鼓励您报告问题并作出更改

* [开发者代码贡献指南](https://github.com/linuxdeepin/developer-center/wiki/Contribution-Guidelines-for-Developers)

## 协议

DTK工具包遵循协议 [LGPL-3.0-or-later](LICENSE).
