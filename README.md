# Development Tool Kit AI

Qt-based development library for AI on Deepin.

中文说明：[README.zh_CN.md](./README.zh_CN.md)

## Dependencies

### Build dependencies

-  cmake,
-  pkg-config,
-  qhelpgenerator-qt5 | qttools5-dev-tools,
-  qtbase5-dev,
-  qttools5-dev,
-  libdtkcore-dev

## Build and install

### Build from source code

1. Make sure you have installed all dependencies.

   ```bash
   git clone https://github.com/linuxdeepin/dtkai
   cd dtkai
   sudo apt build-dep ./
   ```

2. Build

   ```bash
   cmake -B build -DCMAKE_INSTALL_PREFIX=/usr
   cmake --build build
   ```

### Install

```bash
sudo cmake --build build --target install
```

### Runtime dependencies

- deepin-ai-daemon (>= 2.0.0)

## Getting help

Any usage issues can ask for help via

* [Telegram group](https://t.me/deepin)
* [Matrix](https://matrix.to/#/#deepin-community:matrix.org)
* [IRC (libera.chat)](https://web.libera.chat/#deepin-community)
* [Forum](https://bbs.deepin.org)
* [WiKi](https://wiki.deepin.org/)

## Getting involved

We encourage you to report issues and contribute changes

* [Contribution guide for developers](https://github.com/linuxdeepin/developer-center/wiki/Contribution-Guidelines-for-Developers-en).

## License

Development Tool Kit is licensed under [LGPL-3.0-or-later](LICENSE).
