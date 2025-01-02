// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <iostream>

#include <QCoreApplication>
#include <QEventLoop>

#include <DChatCompletions>

DAI_USE_NAMESPACE

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    DChatCompletions chat;

    QString response;
    QEventLoop loop;
    QObject::connect(&chat, &DChatCompletions::streamFinished, &loop, &QEventLoop::quit);
    QObject::connect(&chat, &DChatCompletions::streamOutput, [&chat, &response](const QString &text){
        std::cout << text.toStdString();
        response.append(text);
        std::flush(std::cout);
    });

    QList<ChatHistory> historys;
    std::string prompt;

    for (;;) {
        response.clear();

        std::cout << "question:\n";
        std::getline(std::cin, prompt);

        if (prompt == "Stop")
            break;

        if (prompt == "Clear") {
            historys.clear();
            continue;
        }
        QString content = QString::fromStdString(prompt);
        if (chat.chatStream(content, historys)) {
            loop.exec();

            ChatHistory line;
            line.role = kChatRoleUser;
            line.content = content;
            historys.append(line);

            line.role = kChatRoleAssistant;
            line.content = response;
            historys.append(line);
        } else {
            std::cerr << "error:" << chat.lastError().getErrorCode() << std::endl;
            return chat.lastError().getErrorCode();
        }

        std::cout << "\n----------\n";
    }

    return 0;
}
