// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <DSpeechToText>
#include <DTextToSpeech>
#include <DChatCompletions>
#include <DImageRecognition>
#include <DOCRRecognition>
#include <DAIError>

#include <QCoreApplication>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QTimer>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QAudioSource>
#include <QAudioSink>
#include <QMediaDevices>
#include <QAudioDevice>
#else
#include <QAudioInput>
#include <QAudioOutput>
#include <QAudioDeviceInfo>
#endif

#include <QAudioFormat>
#include <QIODevice>
#include <QBuffer>
#include <QThread>
#include <QEventLoop>
#include <QVariantHash>
#include <QList>
#include <QDateTime>

#include <iostream>
#include <algorithm>

DCORE_USE_NAMESPACE
DAI_USE_NAMESPACE

void waitReturn() {
    std::cout << "\n按回车返回主菜单...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

void nlp_demo() {
    DChatCompletions chat;

    QList<ChatHistory> historys;
    std::string prompt;
    std::cout << "\n【NLP Demo】输入问题，输入 clear 清空历史，stop 返回主菜单\n";

    QString response;
    QEventLoop loop;

    QObject::connect(&chat, &DChatCompletions::streamOutput,
                     [&response](const QString &text) {
        std::cout << text.toStdString() << std::flush;
        response.append(text);
    });

    QObject::connect(&chat, &DChatCompletions::streamFinished,
                     &loop, &QEventLoop::quit);

    while (true) {
        response.clear();

        std::cout << "Q: ";
        if (!std::getline(std::cin, prompt))
            break;

        std::string lowerPrompt = prompt;
        std::transform(lowerPrompt.begin(), lowerPrompt.end(), lowerPrompt.begin(), ::tolower);

        if (lowerPrompt == "stop") break;
        if (lowerPrompt == "clear") {
            historys.clear();
            std::cout << "历史已清空\n";
            continue;
        }
        if (prompt.empty())
            continue;

        QString content = QString::fromStdString(prompt);

        std::cout << "A: ";

        if (chat.chatStream(content, historys)) {
            loop.exec();

            ChatHistory userLine;
            userLine.role = kChatRoleUser;
            userLine.content = content;
            historys.append(userLine);

            ChatHistory assistantLine;
            assistantLine.role = kChatRoleAssistant;
            assistantLine.content = response;
            historys.append(assistantLine);
        } else {
            std::cerr << "error:" << chat.lastError().getErrorCode() << std::endl;
            break;
        }

        std::cout << std::endl;
    }
    waitReturn();
}

void stt_demo() {
    DSpeechToText stt;

    QAudioFormat audioFormat;
    audioFormat.setSampleRate(16000);
    audioFormat.setChannelCount(1);

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    audioFormat.setSampleFormat(QAudioFormat::Int16);
    QAudioDevice inputDeviceInfo = QMediaDevices::defaultAudioInput();
    QAudioSource *audioSource = new QAudioSource(inputDeviceInfo, audioFormat);
#else
    audioFormat.setSampleSize(16);
    audioFormat.setCodec("audio/pcm");
    audioFormat.setByteOrder(QAudioFormat::LittleEndian);
    audioFormat.setSampleType(QAudioFormat::SignedInt);
    QAudioDeviceInfo inputDeviceInfo = QAudioDeviceInfo::defaultInputDevice();
    QAudioInput *audioSource = new QAudioInput(inputDeviceInfo, audioFormat);
#endif

    QIODevice *audioDevice = audioSource->start();

    if (!audioDevice) {
        std::cout << "无法打开麦克风设备！" << std::endl;
        delete audioSource;
        waitReturn();
        return;
    }

    QVariantHash params;
    params["language"] = "zh-cn";
    params["format"] = "pcm";
    params["sampleRate"] = 16000;
    params["channels"] = 1;
    params["bitsPerSample"] = 16;

    stt.startStreamRecognition(params);
    std::cout << "正在录音（8秒后自动结束），请说话..." << std::endl;

    QEventLoop loop;

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QObject::connect(audioSource, &QAudioSource::stateChanged, [&](QAudio::State state){
        if (state == QAudio::StoppedState || state == QAudio::IdleState)
            loop.quit();
    });
#else
    QObject::connect(audioSource, &QAudioInput::stateChanged, [&](QAudio::State state){
        if (state == QAudio::StoppedState || state == QAudio::IdleState)
            loop.quit();
    });
#endif

    QObject::connect(audioDevice, &QIODevice::readyRead, [&](){
        QByteArray data = audioDevice->readAll();
        if (!data.isEmpty()) {
            stt.sendAudioData(data);
        }
    });

    QTimer::singleShot(8000, &loop, [&](){
        audioSource->stop();
    });

    loop.exec();

    QString result = stt.endStreamRecognition();
    std::cout << "识别结果：" << result.toStdString() << std::endl;

    audioSource->stop();
    delete audioSource;
    waitReturn();
}

void tts_demo() {
    DTextToSpeech tts;
    std::cout << "\n【语音合成 Demo】请输入要合成的文本：";
    std::string text;
    if (!std::getline(std::cin, text) || text.empty())
        text = "你好，有什么可以帮您！";

    QVariantHash params;
    params["voice"] = "x4_yezi";
    params["speed"] = 50;
    params["volume"] = 50;

    QAudioFormat audioFormat;
    audioFormat.setSampleRate(16000);
    audioFormat.setChannelCount(1);

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    audioFormat.setSampleFormat(QAudioFormat::Int16);
    QAudioDevice outputDeviceInfo = QMediaDevices::defaultAudioOutput();
    QAudioSink *audioSink = new QAudioSink(outputDeviceInfo, audioFormat);
#else
    audioFormat.setSampleSize(16);
    audioFormat.setCodec("audio/pcm");
    audioFormat.setByteOrder(QAudioFormat::LittleEndian);
    audioFormat.setSampleType(QAudioFormat::SignedInt);
    QAudioDeviceInfo outputDeviceInfo = QAudioDeviceInfo::defaultOutputDevice();
    QAudioOutput *audioSink = new QAudioOutput(outputDeviceInfo, audioFormat);
#endif

    bool synthesisCompleted = false;
    QByteArray accumulatedAudioData;
    QEventLoop syncLoop;

    QObject::connect(&tts, &DTextToSpeech::synthesisCompleted,
                     [&](const QByteArray &audioData) {
                         accumulatedAudioData = audioData;
                         synthesisCompleted = true;
                         syncLoop.quit();
                     });

    QObject::connect(&tts, &DTextToSpeech::synthesisError,
                     [&](int errorCode, const QString &errorMessage) {
                         std::cout << "语音合成错误 [" << errorCode << "]: " << errorMessage.toStdString() << std::endl;
                         synthesisCompleted = true;
                         syncLoop.quit();
                     });

    std::cout << "开始语音合成..." << std::endl;
    if (!tts.startStreamSynthesis(QString::fromStdString(text), params)) {
        auto error = tts.lastError();
        std::cout << "启动失败: " << error.getErrorMessage().toStdString() << std::endl;
        if (audioSink)
            delete audioSink;
        waitReturn();
        return;
    }

    if (!synthesisCompleted)
        syncLoop.exec();

    if (accumulatedAudioData.isEmpty()) {
        std::cout << "未获取音频数据。" << std::endl;
        if (audioSink) delete audioSink;
        waitReturn();
        return;
    }

    QBuffer *audioBuffer = new QBuffer();
    audioBuffer->setData(accumulatedAudioData);
    audioBuffer->open(QIODevice::ReadOnly);

    QEventLoop playLoop;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QObject::connect(audioSink, &QAudioSink::stateChanged, [&](QAudio::State state) {
        if (state == QAudio::IdleState || state == QAudio::StoppedState) playLoop.quit();
    });
#else
    QObject::connect(audioSink, &QAudioOutput::stateChanged, [&](QAudio::State state) {
        if (state == QAudio::IdleState || state == QAudio::StoppedState) playLoop.quit();
    });
#endif

    std::cout << "开始播放..." << std::endl;
    audioSink->start(audioBuffer);
    playLoop.exec();

    // 停止设备并关闭缓冲区，防止 delete 时后端仍在访问
    if (audioSink)
        audioSink->stop();
    audioBuffer->close();

    delete audioBuffer;
    delete audioSink;

    waitReturn();
}

void vision_demo() {
    DImageRecognition vision;
    std::cout << "\n【图像识别 Demo】请输入图片路径：";
    std::string path;
    if (!std::getline(std::cin, path)) return;
    QString imagePath = QString::fromStdString(path).trimmed();
    if (!QFile::exists(imagePath)) {
        std::cout << "文件不存在。" << std::endl;
        waitReturn();
        return;
    }
    QString prompt = "请详细描述这张图片的内容。";
    QString result = vision.recognizeImage(imagePath, prompt);
    if (vision.lastError().getErrorCode() != NoError) {
        std::cout << "失败: " << vision.lastError().getErrorMessage().toStdString() << std::endl;
    } else {
        std::cout << "结果：" << result.toStdString() << std::endl;
    }
    waitReturn();
}

void ocr_demo() {
    DOCRRecognition ocr;
    std::cout << "\n【OCR文字识别 Demo】请输入图片路径：";
    std::string path;
    if (!std::getline(std::cin, path)) return;
    QString imagePath = QString::fromStdString(path).trimmed();

    if (!QFile::exists(imagePath)) {
        std::cout << "文件不存在。" << std::endl;
        waitReturn();
        return;
    }

    QVariantHash params;
    params["language"] = "zh-Hans_en";

    std::cout << "识别中..." << std::endl;
    QString result = ocr.recognizeFile(imagePath, params);
    if (ocr.lastError().getErrorCode() != NoError) {
        std::cout << "失败: " << ocr.lastError().getErrorMessage().toStdString() << std::endl;
    } else {
        std::cout << "结果：\n" << result.toStdString() << std::endl;
    }
    waitReturn();
}

void print_menu() {
    std::cout << "\n========= DTK AI Demo (Qt5/Qt6 Dual) =========\n";
    std::cout << "1. NLP 问答\n";
    std::cout << "2. 语音识别 (STT)\n";
    std::cout << "3. 语音合成 (TTS)\n";
    std::cout << "4. 图像识别\n";
    std::cout << "5. OCR文字识别\n";
    std::cout << "0. 退出\n";
    std::cout << "选择功能: ";
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    while (true) {
        print_menu();
        std::string choice;
        if (!std::getline(std::cin, choice)) break;
        if (choice == "1") nlp_demo();
        else if (choice == "2") stt_demo();
        else if (choice == "3") tts_demo();
        else if (choice == "4") vision_demo();
        else if (choice == "5") ocr_demo();
        else if (choice == "0") break;
    }
    return 0;
}
