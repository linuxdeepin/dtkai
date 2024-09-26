// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <iostream>

#include <QCoreApplication>

#include <DFunctionCalling>

#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>

DAI_USE_NAMESPACE
const char *functionList= R"(
                          [
                              {
                                  "name": "filemanager",
                                  "description": "打开或启动文件管理器、文管",
                                  "parameters": {
                                  "type": "object",
                                  "properties": {
                                      }
                                  }
                              },
                              {
                                  "name": "switchSystemTheme",
                                  "description": "切换或设置系统主题，如：浅色，暗色",
                                  "parameters": {
                                      "type": "object",
                                      "properties": {
                                          "theme": {
                                              "type": "string",
                                              "description": "Light浅色,Dark暗色",
                                              "enum": ["Light", "Dark"]
                                          }
                                      },
                                      "required": ["theme"]
                                  }
                              }
                            ]
                          )";

static void doFunction(const QVariantHash &function)
{
    QString name = function.value("name").toString();
    if (name == QString("filemanager")) {
        QProcess::startDetached("gio", QStringList({"open", "computer://"}));
    } else if (name == QString("switchSystemTheme")){
         QString arg = function.value("arguments").toString();
         QVariantHash args = QJsonDocument::fromJson(arg.toUtf8()).object().toVariantHash();
         QString theme = args.value("theme").toString();
         if (theme == "Light") {
             QProcess::startDetached("qdbus", QStringList({"com.deepin.daemon.Appearance",
                                                           "/com/deepin/daemon/Appearance", "com.deepin.daemon.Appearance.Set"
                                                          , "gtk", "deepin"}));
         } else if (theme == "Dark") {
             QProcess::startDetached("qdbus", QStringList({"com.deepin.daemon.Appearance",
                                                           "/com/deepin/daemon/Appearance", "com.deepin.daemon.Appearance.Set"
                                                          , "gtk", "deepin-dark"}));
         }
    }
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    DFunctionCalling funcIfs;
    QString response;
    std::string prompt;

    for (;;) {
        response.clear();

        std::cout << "question:\n";
        // 1. 打开文管
        // 2. 切换深色主题
        std::getline(std::cin, prompt);

        if (prompt == "Stop")
            break;

        QString output = funcIfs.parse(QString::fromStdString(prompt), functionList);
        if (funcIfs.lastError().getErrorCode() != 0) {
            std::cerr << "error:" << funcIfs.lastError().getErrorCode() << " " <<
                         funcIfs.lastError().getErrorMessage().toStdString() << std::endl;
            return funcIfs.lastError().getErrorCode();
        }

        if (output.isEmpty()) {
            std::cout << "No action to do." << std::endl;
        } else {
            QVariantHash function = QJsonDocument::fromJson(output.toUtf8()).object().toVariantHash();
            std::cout << "Do: " << output.toStdString() << std::endl;
            doFunction(function);
        }

        std::cout << "\n----------\n";
    }

    return 0;
}
