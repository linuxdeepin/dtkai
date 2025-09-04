// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

/**
 * @file presentation_demo.cpp
 * @brief Comprehensive AI Presentation Demo for Deepin System Assistant
 *
 * This demo demonstrates the integration of Speech-to-Text, Text-to-Speech,
 * NLP Chat, and Image Recognition capabilities in a sequential presentation format.
 *
 * Presentation Steps:
 * 1. Welcome and introduction (TTS)
 * 2. NLP demonstration - Deepin desktop background setting (TTS + Chat)
 * 3. Speech recognition demonstration (STT)
 * 4. Vision demonstration - Image analysis (TTS + Vision)
 * 5. Summary of demonstration (TTS)
 */

#include <DSpeechToText>
#include <DTextToSpeech>
#include <DChatCompletions>
#include <DImageRecognition>

#include <QCoreApplication>
#include <QTimer>
#include <QAudioInput>
#include <QAudioFormat>
#include <QAudioDeviceInfo>
#include <QIODevice>
#include <QByteArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QScopedPointer>
#include <QMutex>
#include <QThread>
#include <QEventLoop>
#include <QDateTime>
#include <QAudioOutput>
#include <QBuffer>
#include <QQueue>
#include <QMutexLocker>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <functional>

DCORE_USE_NAMESPACE
DAI_USE_NAMESPACE

// Task definition for the presentation queue
struct PresentationTask {
    QString taskId;
    QString description;
    std::function<void()> executeFunction;
    std::function<bool()> completionChecker;
    
    PresentationTask(const QString &id, const QString &desc, 
                    std::function<void()> exec, std::function<bool()> checker)
        : taskId(id), description(desc), executeFunction(exec), completionChecker(checker) {}
};

class PresentationDemo : public QObject
{
    Q_OBJECT

public:
    PresentationDemo(QObject *parent = nullptr);
    ~PresentationDemo();

    void startPresentation();

private Q_SLOTS:
    // Speech-to-Text slots
    void onRecognitionResult(const QString &text);
    void onRecognitionPartialResult(const QString &partialText);
    void onRecognitionError(int errorCode, const QString &errorMessage);
    void onRecognitionCompleted(const QString &finalText);

    // Chat slots
    void onStreamOutput(const QString &content);
    void onStreamFinished(int error);

    // Text-to-Speech slots
    void onSynthesisResult(const QByteArray &audioData);
    void onSynthesisError(int errorCode, const QString &errorMessage);
    void onSynthesisCompleted(const QByteArray &finalAudio);

    // Audio playback slots
    void onAudioStateChanged(QAudio::State state);

    // Audio input slots
    void onAudioDataReady();

private:
    // Task queue management methods
    void initializeTaskQueue();
    void executeNextTask();
    void checkTaskCompletion();
    void completeCurrentTask();
    
    // Presentation flow methods
    void speakText(const QString &text);
    void waitForSpeechComplete();

    // Individual step implementations
    void step1_Welcome();
    void step2_NLPDemo();
    void step3_SpeechDemo();
    void step4_VisionDemo();
    void step5_Summary();

    // Utility methods
    void setupAudioInput();
    void setupAudioOutput();
    void startRecording();
    void stopRecording();
    void playAudioFile(const QString &filename);
    void saveAudioToFile(const QByteArray &audioData, const QString &filename);
    void displayStatus(const QString &module, const QString &status, const QString &details = "");
    void displayResult(const QString &module, const QString &result);

private:
    // AI services
    DSpeechToText *stt = nullptr;
    DChatCompletions *chat = nullptr;
    DTextToSpeech *tts = nullptr;
    DImageRecognition *imageRecognition = nullptr;

    // Audio input
    QAudioInput *audioInput = nullptr;
    QIODevice *audioDevice = nullptr;
    QAudioFormat audioFormat;
    bool isRecording = false;
    QString currentUserInput;

    // Audio output
    QAudioOutput *audioOutput = nullptr;
    QBuffer *audioBuffer = nullptr;
    QByteArray accumulatedAudioData; // Buffer for streaming audio

    // Task queue system
    QQueue<PresentationTask> taskQueue;
    PresentationTask *currentTask = nullptr;
    bool isTaskRunning = false;
    QTimer *taskCompletionTimer = nullptr;
    
    // Presentation state
    bool isPresentationRunning = false;
    bool isWaitingForSpeech = false;
    QString currentSpeechText;
    QString outputFilename;

    // Task completion tracking
    bool speechCompleted = false;
    bool chatCompleted = false;
    bool recognitionCompleted = false;
    bool visionCompleted = false;
    bool isAudioPlaying = false;
    bool isSynthesisCompleted = false;  // 标记语音合成是否完成

    // Timers
    QTimer *stepTimer = nullptr;
    QTimer *recognitionTimeoutTimer = nullptr;
    QTimer *idleConfirmationTimer = nullptr;  // 用于确认音频播放真正完成的定时器
    static const int RECOGNITION_TIMEOUT_MS = 5000; // 15 seconds timeout
    static const int IDLE_CONFIRMATION_MS = 200; // 2 seconds to confirm idle state
    static const int STEP_DELAY_MS = 200; // 2 seconds delay between steps
};

PresentationDemo::PresentationDemo(QObject *parent)
    : QObject(parent)
{
    qInfo() << "🤖 Creating Deepin AI Presentation Demo...";

    // Create AI service instances
    stt = new DSpeechToText(this);
    chat = new DChatCompletions(this);
    tts = new DTextToSpeech(this);
    imageRecognition = new DImageRecognition(this);

    // Connect Speech-to-Text signals
    connect(stt, &DSpeechToText::recognitionResult,
            this, &PresentationDemo::onRecognitionResult);
    connect(stt, &DSpeechToText::recognitionPartialResult,
            this, &PresentationDemo::onRecognitionPartialResult);
    connect(stt, &DSpeechToText::recognitionError,
            this, &PresentationDemo::onRecognitionError);
    connect(stt, &DSpeechToText::recognitionCompleted,
            this, &PresentationDemo::onRecognitionCompleted);

    // Connect Chat signals
    connect(chat, &DChatCompletions::streamOutput,
            this, &PresentationDemo::onStreamOutput);
    connect(chat, &DChatCompletions::streamFinished,
            this, &PresentationDemo::onStreamFinished);

    // Connect Text-to-Speech signals
    connect(tts, &DTextToSpeech::synthesisResult,
            this, &PresentationDemo::onSynthesisResult);
    connect(tts, &DTextToSpeech::synthesisError,
            this, &PresentationDemo::onSynthesisError);
    connect(tts, &DTextToSpeech::synthesisCompleted,
            this, &PresentationDemo::onSynthesisCompleted);

    // Setup audio format (16kHz, 16-bit, mono)
    audioFormat.setSampleRate(16000);
    audioFormat.setChannelCount(1);
    audioFormat.setSampleSize(16);
    audioFormat.setCodec("audio/pcm");
    audioFormat.setByteOrder(QAudioFormat::LittleEndian);
    audioFormat.setSampleType(QAudioFormat::SignedInt);

    // Create timers
    taskCompletionTimer = new QTimer(this);
    taskCompletionTimer->setSingleShot(false);
    taskCompletionTimer->setInterval(1000); // Check every second
    connect(taskCompletionTimer, &QTimer::timeout, this, &PresentationDemo::checkTaskCompletion);

    recognitionTimeoutTimer = new QTimer(this);
    recognitionTimeoutTimer->setSingleShot(true);
    connect(recognitionTimeoutTimer, &QTimer::timeout, [this]() {
        if (isRecording) {
            displayStatus("📝 Speech-to-Text", "⏰ Timeout", "Using latest partial result");
            if (!currentUserInput.isEmpty()) {
                displayResult("📝 Speech-to-Text", currentUserInput);
            }
            stopRecording();
            recognitionCompleted = true;
        }
    });

    idleConfirmationTimer = new QTimer(this);
    idleConfirmationTimer->setSingleShot(true);
    connect(idleConfirmationTimer, &QTimer::timeout, [this]() {
        // 延迟确认：如果语音合成已完成且仍然没有音频播放，则认为真正完成
        if (isSynthesisCompleted && !isAudioPlaying) {
            qInfo() << "🎤 Idle confirmation: Speech synthesis and playback both completed";
            speechCompleted = true;
            isWaitingForSpeech = false;
        } else {
            qInfo() << "🎤 Idle confirmation: Still waiting - synthesis:" << isSynthesisCompleted 
                    << ", audioPlaying:" << isAudioPlaying;
        }
    });

    qInfo() << "✅ Deepin AI Presentation Demo created successfully";
}

PresentationDemo::~PresentationDemo()
{
    if (audioInput) {
        audioInput->stop();
    }
    if (audioOutput) {
        audioOutput->stop();
    }

    qInfo() << "✅ Presentation Demo destroyed";
}

void PresentationDemo::startPresentation()
{
    qInfo() << "\n" << QString(80, '=');
    qInfo() << "🎤 Deepin AI Assistant - Comprehensive Presentation Demo";
    qInfo() << QString(80, '=');
    qInfo() << "🏠 Role: Deepin System Computer Manager Assistant";
    qInfo() << "📋 Presentation Steps:";
    qInfo() << "   1. 👋 Welcome and Introduction";
    qInfo() << "   2. 🤖 NLP Demo - Deepin Desktop Background Setting";
    qInfo() << "   3. 🎤 Speech Recognition Demo";
    qInfo() << "   4. 👁️  Vision Demo - Image Analysis";
    qInfo() << "   5. 📝 Presentation Summary";
    qInfo() << QString(80, '=');
    qInfo() << "💡 Each step will complete before moving to the next...";
    qInfo() << "⏹️  Press Ctrl+C to stop the demo";
    qInfo() << QString(80, '=') << "\n";

    setupAudioInput();
    setupAudioOutput();
    
    // Initialize task queue
    initializeTaskQueue();

    isPresentationRunning = true;
    
    // Start executing tasks
    executeNextTask();
}

void PresentationDemo::initializeTaskQueue()
{
    qInfo() << "🔧 Initializing task queue...";

    // Task 1: Welcome and Introduction
    taskQueue.enqueue(PresentationTask(
        "welcome",
        "Welcome and Introduction",
        [this]() {
            qInfo() << "\n🎯 Executing Task: Welcome and Introduction";
            qInfo() << QString(40, '-');
            speechCompleted = false;
            isAudioPlaying = false;
            isSynthesisCompleted = false;
            step1_Welcome();
        },
        [this]() { 
            // 确保语音合成完成且音频播放也完成
            return speechCompleted && !isAudioPlaying && !isWaitingForSpeech; 
        }
    ));
    
    // Task 2: NLP Demo Introduction
    taskQueue.enqueue(PresentationTask(
        "nlp_intro",
        "NLP Demo Introduction",
        [this]() {
            qInfo() << "\n🎯 Executing Task: NLP Demo Introduction";
            qInfo() << QString(40, '-');
            speechCompleted = false;
            isAudioPlaying = false;
            isSynthesisCompleted = false;
            QString introText = "接下来我将演示自然语言处理功能。"
                               "我可以回答关于deepin系统的各种问题。"
                               "现在让我来回答一个常见问题：如何设置桌面背景。";
            displayStatus("🤖 Step 2", "🔊 Speaking", "NLP Demo Introduction");
            speakText(introText);
        },
        [this]() { 
            // 确保语音合成完成且音频播放也完成
            return speechCompleted && !isAudioPlaying && !isWaitingForSpeech; 
        }
    ));
    
    // Task 3: NLP Processing and Response
    taskQueue.enqueue(PresentationTask(
        "nlp_processing",
        "NLP Processing and Response",
        [this]() {
            qInfo() << "\n🎯 Executing Task: NLP Processing and Response";
            qInfo() << QString(40, '-');
            speechCompleted = false;
            chatCompleted = false;
            isAudioPlaying = false;
            isSynthesisCompleted = false;
            
            QString question = "deepin系统如何设置桌面背景？";
            displayStatus("🤖 NLP Processing", "🔄 Processing", question);

            currentSpeechText = "要设置deepin系统的桌面背景，您可以按照以下步骤操作：\n"
                               "第一步，右键点击桌面空白区域，选择\"个性化\"选项。\n"
                               "第二步，在打开的窗口中，您可以选择系统预设的壁纸。"
                               "这样就成功设置了您的桌面背景。";

            QTimer::singleShot(200, [this]() {
                displayResult("🤖 NLP Response", currentSpeechText);
                chatCompleted = true;
                speakText(currentSpeechText);
            });
        },
        [this]() { 
            // 确保NLP处理完成、语音合成完成且音频播放也完成
            return speechCompleted && chatCompleted && !isAudioPlaying && !isWaitingForSpeech; 
        }
    ));
    
    // Task 4: Speech Recognition Demo Introduction
    taskQueue.enqueue(PresentationTask(
        "speech_intro",
        "Speech Recognition Demo Introduction",
        [this]() {
            qInfo() << "\n🎯 Executing Task: Speech Recognition Demo Introduction";
            qInfo() << QString(40, '-');
            speechCompleted = false;
            isAudioPlaying = false;
            isSynthesisCompleted = false;
            QString introText = "现在我将演示语音识别功能。"
                               "请您对着麦克风说一段话，"
                               "我将把您的语音转换成文字显示出来。"
                               "我现在开始监听您的语音输入。";
            displayStatus("🎤 Step 3", "🔊 Speaking", "Speech Recognition Demo");
            speakText(introText);
        },
        [this]() { 
            // 确保语音合成完成且音频播放也完成
            return speechCompleted && !isAudioPlaying && !isWaitingForSpeech; 
        }
    ));

    // Task 5: Speech Recognition Processing
    taskQueue.enqueue(PresentationTask(
        "speech_recognition",
        "Speech Recognition Processing",
        [this]() {
            qInfo() << "\n🎯 Executing Task: Speech Recognition Processing";
            qInfo() << QString(40, '-');
            recognitionCompleted = false;
            startRecording();
        },
        [this]() { return recognitionCompleted; }
    ));
    
    // Task 6: Vision Demo Introduction
    taskQueue.enqueue(PresentationTask(
        "vision_intro",
        "Vision Demo Introduction",
        [this]() {
            qInfo() << "\n🎯 Executing Task: Vision Demo Introduction";
            qInfo() << QString(40, '-');
            speechCompleted = false;
            isAudioPlaying = false;
            isSynthesisCompleted = false;
            QString introText = "接下来我将演示图像识别功能。"
                               "我将分析指定的图片，"
                               "然后告诉您图片中包含的内容。"
                               "让我来看看这张图片。";
            displayStatus("👁️ Step 4", "🔊 Speaking", "Vision Demo Introduction");
            speakText(introText);
        },
        [this]() { 
            // 确保语音合成完成且音频播放也完成
            return speechCompleted && !isAudioPlaying && !isWaitingForSpeech; 
        }
    ));
    
    // Task 7: Vision Processing
    taskQueue.enqueue(PresentationTask(
        "vision_processing",
        "Vision Processing",
        [this]() {
            qInfo() << "\n🎯 Executing Task: Vision Processing";
            qInfo() << QString(40, '-');
            speechCompleted = false;
            visionCompleted = false;
            isAudioPlaying = false;
            isSynthesisCompleted = false;
            
            QString imagePath = "/home/ut000824@uos/Desktop/temp/images.jpeg";
            QString prompt = "请详细描述这张图片的内容，包括看到了什么物体、场景、人物或文字等。";
            
            displayStatus("👁️ Vision Processing", "🔄 Analyzing", imagePath);
            
            QString result = imageRecognition->recognizeImage(imagePath, prompt);
            auto error = imageRecognition->lastError();
            
            if (error.getErrorCode() != -1) {
                qWarning() << "图像识别失败:" << error.getErrorCode() << error.getErrorMessage();
                currentSpeechText = "很抱歉，我无法分析这张图片。"
                                   "可能是图片文件不存在或者图像识别服务暂时不可用。"
                                   "在正常情况下，我可以识别图片中的物体、场景、文字等内容，"
                                   "并为您提供详细的描述。";
            } else if (result.isEmpty()) {
                currentSpeechText = "图片分析完成，但是没有获得有效的识别结果。"
                                   "这可能是因为图片内容比较复杂或者光线条件不佳。";
            } else {
                currentSpeechText = QString("通过图像识别分析，我看到了以下内容：%1").arg(result);
            }
            
            displayResult("👁️ Vision Analysis", currentSpeechText);
            visionCompleted = true;
            speakText(currentSpeechText);
        },
        [this]() { 
            // 确保视觉处理完成、语音合成完成且音频播放也完成
            return speechCompleted && visionCompleted && !isAudioPlaying && !isWaitingForSpeech; 
        }
    ));
    
    // Task 8: Summary
    taskQueue.enqueue(PresentationTask(
        "summary",
        "Presentation Summary",
        [this]() {
            qInfo() << "\n🎯 Executing Task: Presentation Summary";
            qInfo() << QString(40, '-');
            speechCompleted = false;
            isAudioPlaying = false;
            isSynthesisCompleted = false;
            QString summaryText = "今天的演示到此结束。"
                                 "我为大家展示了deepin系统AI助手的四大核心功能：\n"
                                 "第一，自然语言处理功能，我可以理解并回答各种关于deepin系统的问题。"
                                 "第二，语音识别功能，我能够准确地将您的语音转换成文字。"
                                 "第三，图像识别功能，我可以分析图片内容并提供详细描述。"
                                 "第四，语音合成功能，我能够将文字转换成自然的语音。"
                                 "作为deepin系统的AI电脑管家，"
                                 "我将竭诚为用户提供智能、便捷的系统管理和技术支持服务。"
                                 "谢谢大家观看今天的演示！";
            displayStatus("📝 Step 5", "🔊 Speaking", "Presentation Summary");
            speakText(summaryText);
        },
        [this]() { 
            // 确保语音合成完成且音频播放也完成
            return speechCompleted && !isAudioPlaying && !isWaitingForSpeech; 
        }
    ));
    
    qInfo() << "✅ Task queue initialized with" << taskQueue.size() << "tasks";
}

void PresentationDemo::executeNextTask()
{
    if (!isPresentationRunning) return;
    
    if (taskQueue.isEmpty()) {
        qInfo() << "🎉 All tasks completed! Presentation finished successfully!";
        qInfo() << "👋 Thank you for watching the Deepin AI Assistant demo!";
        isPresentationRunning = false;
        taskCompletionTimer->stop();
        QTimer::singleShot(200, qApp, &QCoreApplication::quit);
        return;
    }
    
    if (isTaskRunning) {
        qWarning() << "⚠️  Task already running, skipping execution";
        return;
    }
    
    // Get next task from queue
    PresentationTask task = taskQueue.dequeue();
    currentTask = new PresentationTask(task);
    isTaskRunning = true;
    
    qInfo() << "🚀 Starting task:" << currentTask->taskId << "-" << currentTask->description;
    
    // Execute the task
    currentTask->executeFunction();
    
    // Start monitoring task completion
    taskCompletionTimer->start();
}

void PresentationDemo::checkTaskCompletion()
{
    if (!isTaskRunning || !currentTask) return;
    
    // Check if current task is completed
    if (currentTask->completionChecker()) {
        completeCurrentTask();
    }
}

void PresentationDemo::completeCurrentTask()
{
    if (!currentTask) return;
    
    qInfo() << "✅ Task completed:" << currentTask->taskId << "-" << currentTask->description;
    
    // Clean up current task
    delete currentTask;
    currentTask = nullptr;
    isTaskRunning = false;
    taskCompletionTimer->stop();
    
    // Wait a bit before starting next task
    QTimer::singleShot(STEP_DELAY_MS, this, &PresentationDemo::executeNextTask);
}

void PresentationDemo::step1_Welcome()
{
    QString welcomeText = "大家好！我是deepin系统的AI电脑管家。"
                         "我可以帮助用户解决各种系统问题，"
                         "包括系统设置、故障诊断、使用指导等。"
                         "今天我将为大家演示我的核心功能，"
                         "包括自然语言处理、语音识别和图像识别能力。"
                         "让我们开始这次演示吧！";

    displayStatus("👋 Step 1", "🔊 Speaking", "Welcome and Introduction");
    speakText(welcomeText);
}

// step2_NLPDemo is now integrated into the task queue system

// step3_SpeechDemo, step4_VisionDemo, and step5_Summary are now integrated into the task queue system

void PresentationDemo::speakText(const QString &text)
{
    if (!tts || text.isEmpty()) return;

    isWaitingForSpeech = true;
    isSynthesisCompleted = false;  // 重置语音合成完成标志
    currentSpeechText = text;
    
    // Clear previous audio data
    accumulatedAudioData.clear();
    outputFilename.clear();

    displayStatus("🔊 Text-to-Speech", "🔄 Synthesizing",
                  QString("Text length: %1 characters").arg(text.length()));

    qInfo() << "🎤 Starting stream synthesis for:" << text.left(50) + "...";
    
    bool result = tts->startStreamSynthesis(text);
    if (!result) {
        auto error = tts->lastError();
        qCritical() << "❌ Failed to start TTS synthesis:" << error.getErrorCode() << error.getErrorMessage();
        
        // Fallback: mark as completed without audio
        speechCompleted = true;
        isWaitingForSpeech = false;
    } else {
        qInfo() << "✅ TTS synthesis started successfully";
    }
}

void PresentationDemo::setupAudioInput()
{
    if (audioInput) {
        audioInput->stop();
        delete audioInput;
        audioInput = nullptr;
    }

    QAudioDeviceInfo selectedDevice = QAudioDeviceInfo::defaultInputDevice();

    if (!selectedDevice.isFormatSupported(audioFormat)) {
        qWarning() << "⚠️  Audio format not supported, using nearest format...";
        audioFormat = selectedDevice.nearestFormat(audioFormat);
    }

    audioInput = new QAudioInput(selectedDevice, audioFormat, this);
    audioInput->setVolume(0.7);

    qInfo() << "✅ Audio input setup completed";
}

void PresentationDemo::setupAudioOutput()
{
    if (audioOutput) {
        audioOutput->stop();
        delete audioOutput;
        audioOutput = nullptr;
    }

    if (audioBuffer) {
        delete audioBuffer;
        audioBuffer = nullptr;
    }

    QAudioDeviceInfo selectedDevice = QAudioDeviceInfo::defaultOutputDevice();
    audioOutput = new QAudioOutput(selectedDevice, audioFormat, this);
    audioOutput->setVolume(0.8);

    audioBuffer = new QBuffer(this);
    audioBuffer->open(QIODevice::ReadWrite);

    connect(audioOutput, &QAudioOutput::stateChanged,
            this, &PresentationDemo::onAudioStateChanged);

    qInfo() << "✅ Audio output setup completed";
}

void PresentationDemo::startRecording()
{
    if (!stt) return;

    displayStatus("🎤 Speech Recognition", "🔄 Starting", "Listening for speech...");

    currentUserInput.clear();

    QVariantHash params;
    params["language"] = "zh-cn";
    params["format"] = "pcm";
    params["sampleRate"] = audioFormat.sampleRate();
    params["channels"] = audioFormat.channelCount();
    params["bitsPerSample"] = audioFormat.sampleSize();
    params["vad_eos"] = 2000; // 2 seconds of silence to end recognition

    stt->startStreamRecognition(params);
    isRecording = true;

    // Create audio device
    audioDevice = audioInput->start();
    if (audioDevice) {
        connect(audioDevice, &QIODevice::readyRead,
                this, &PresentationDemo::onAudioDataReady);
    }

    recognitionTimeoutTimer->start(RECOGNITION_TIMEOUT_MS);
}

void PresentationDemo::stopRecording()
{
    if (!stt || !isRecording) return;

    isRecording = false;
    recognitionTimeoutTimer->stop();

    displayStatus("🎤 Speech Recognition", "⏹️  Stopping", "Processing final audio");

    QString finalText = stt->endStreamRecognition();
    if (!finalText.isEmpty()) {
        displayResult("📝 Speech-to-Text", finalText);
        currentUserInput = finalText;
    }

    if (audioInput) {
        audioInput->stop();
    }
}

void PresentationDemo::onAudioDataReady()
{
    if (!isRecording || !audioDevice) return;

    QByteArray audioData = audioDevice->readAll();
    if (!audioData.isEmpty()) {
        stt->sendAudioData(audioData);
    }
}

void PresentationDemo::onRecognitionPartialResult(const QString &partialText)
{
    if (!partialText.isEmpty()) {
        displayStatus("📝 Speech-to-Text", "🔄 Processing", partialText);
        currentUserInput = partialText;
    }
}

void PresentationDemo::onRecognitionResult(const QString &text)
{
    if (!text.isEmpty()) {
        displayResult("📝 Speech-to-Text", text);
        currentUserInput = text;
        stopRecording();
        recognitionCompleted = true;
    }
}

void PresentationDemo::onRecognitionError(int errorCode, const QString &errorMessage)
{
    displayStatus("🎤 Speech Recognition", "❌ Error",
                  QString("Code: %1, Message: %2").arg(errorCode).arg(errorMessage));
    stopRecording();
    recognitionCompleted = true; // Mark as completed even on error
}

void PresentationDemo::onRecognitionCompleted(const QString &finalText)
{
    displayStatus("🎤 Speech Recognition", "✅ Completed", finalText);
    if (!finalText.isEmpty()) {
        currentUserInput = finalText;
    }
    recognitionCompleted = true;
}

void PresentationDemo::onStreamOutput(const QString &content)
{
    displayStatus("🤖 AI Chat", "🔄 Streaming", content);
}

void PresentationDemo::onStreamFinished(int error)
{
    if (error == 0) {
        chatCompleted = true;
    } else {
        displayStatus("🤖 AI Chat", "❌ Error", QString("Error code: %1").arg(error));
        chatCompleted = true; // Mark as completed even on error
    }
}

void PresentationDemo::onSynthesisResult(const QByteArray &audioData)
{
    if (!audioData.isEmpty()) {
        accumulatedAudioData.append(audioData);
//        qInfo() << "📡 Received audio chunk:" << audioData.size() << "bytes, total:" << accumulatedAudioData.size() << "bytes";
    }
}

void PresentationDemo::onSynthesisError(int errorCode, const QString &errorMessage)
{
    displayStatus("🔊 Text-to-Speech", "❌ Error",
                  QString("Code: %1, Message: %2").arg(errorCode).arg(errorMessage));
    isWaitingForSpeech = false;
    speechCompleted = true; // Mark as completed even on error
}

void PresentationDemo::onSynthesisCompleted(const QByteArray &/*finalAudio*/)
{
    displayStatus("🔊 Text-to-Speech", "✅ Synthesis Completed", "Processing audio data...");

    isSynthesisCompleted = true;  // 标记语音合成已完成
    qInfo() << "🎤 Speech synthesis completed, total audio data:" << accumulatedAudioData.size() << "bytes";

    if (!accumulatedAudioData.isEmpty()) {
        qInfo() << "🎵 Total audio data received:" << accumulatedAudioData.size() << "bytes";
        
        // Save complete audio to file
        saveAudioToFile(accumulatedAudioData, outputFilename);
        
        // Play the complete audio file
        if (!outputFilename.isEmpty()) {
            playAudioFile(outputFilename);
        } else {
            speechCompleted = true;
            isWaitingForSpeech = false;
        }
    } else {
        qWarning() << "⚠️  No audio data received from synthesis";
        speechCompleted = true;
        isWaitingForSpeech = false;
    }
}

void PresentationDemo::playAudioFile(const QString &filename)
{
    qDebug() << "--------------------------------------------\n";
    if (!audioOutput || !audioBuffer) {
        speechCompleted = true;
        isWaitingForSpeech = false;
        return;
    }

    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "⚠️  Failed to open audio file:" << filename;
        speechCompleted = true;
        isWaitingForSpeech = false;
        return;
    }

    QByteArray audioData = file.readAll();
    file.close();

    if (audioData.isEmpty()) {
        qWarning() << "⚠️  Audio file is empty:" << filename;
        speechCompleted = true;
        isWaitingForSpeech = false;
        return;
    }

    // 正确的方式：完全替换缓冲区数据，避免数据污染
    audioBuffer->close();
    audioBuffer->setData(audioData);
    audioBuffer->open(QIODevice::ReadOnly);

    isAudioPlaying = true;  // 标记音频开始播放
    audioOutput->start(audioBuffer);

    displayStatus("🎵 Audio Playback", "🔊 Playing",
                  QString("Size: %1 bytes").arg(audioData.size()));
}

void PresentationDemo::onAudioStateChanged(QAudio::State state)
{
    qDebug() << "+++++++++++++++++++++++++++++++++++++\n" << state;
    switch (state) {
    case QAudio::ActiveState:
        displayStatus("🎵 Audio Playback", "🔊 Active", "Playing audio");
        // 如果音频重新开始播放，取消延迟确认定时器
        if (idleConfirmationTimer->isActive()) {
            idleConfirmationTimer->stop();
            qInfo() << "🎤 Audio resumed, cancelling idle confirmation timer";
        }
        break;
    case QAudio::SuspendedState:
        displayStatus("🎵 Audio Playback", "⏸️  Suspended", "Audio paused");
        break;
    case QAudio::StoppedState:
        displayStatus("🎵 Audio Playback", "⏹️  Stopped", "Audio finished");
//        speechCompleted = true;
        isWaitingForSpeech = false;
        isAudioPlaying = false;  // 清除音频播放标志
        break;
    case QAudio::IdleState:
        displayStatus("🎵 Audio Playback", "💤 Idle", "Audio playback idle");
        isAudioPlaying = false;  // 清除音频播放标志
        
        // 启动延迟确认定时器，避免因播放速度快于合成速度而误判完成
        if (isSynthesisCompleted) {
            qInfo() << "🎤 Audio idle and synthesis completed, confirming completion in" << IDLE_CONFIRMATION_MS << "ms";
            idleConfirmationTimer->start(IDLE_CONFIRMATION_MS);
        } else {
            qInfo() << "🎤 Audio idle but synthesis not completed, waiting for more audio data...";
        }
        break;
    case QAudio::InterruptedState:
        displayStatus("🎵 Audio Playback", "⚠️  Interrupted", "Audio interrupted");
        speechCompleted = true;
        isWaitingForSpeech = false;
        isAudioPlaying = false;  // 清除音频播放标志
        break;
    }
}

void PresentationDemo::saveAudioToFile(const QByteArray &audioData, const QString &/*filename*/)
{
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
    outputFilename = QString("/tmp/presentation_audio_%1.wav").arg(timestamp);

    // Create WAV header for PCM format
    QByteArray wavHeader;
    wavHeader.resize(44);

    // RIFF header
    wavHeader.replace(0, 4, "RIFF");
    qint32 fileSize = audioData.size() + 36;
    wavHeader.replace(4, 4, QByteArray::fromRawData(reinterpret_cast<const char*>(&fileSize), 4));
    wavHeader.replace(8, 4, "WAVE");

    // fmt chunk
    wavHeader.replace(12, 4, "fmt ");
    qint32 fmtSize = 16;
    wavHeader.replace(16, 4, QByteArray::fromRawData(reinterpret_cast<const char*>(&fmtSize), 4));
    qint16 audioFormat = 1; // PCM
    wavHeader.replace(20, 2, QByteArray::fromRawData(reinterpret_cast<const char*>(&audioFormat), 2));
    qint16 numChannels = 1; // Mono
    wavHeader.replace(22, 2, QByteArray::fromRawData(reinterpret_cast<const char*>(&numChannels), 2));
    qint32 sampleRate = 16000;
    wavHeader.replace(24, 4, QByteArray::fromRawData(reinterpret_cast<const char*>(&sampleRate), 4));
    qint32 byteRate = sampleRate * numChannels * 2; // 16-bit
    wavHeader.replace(28, 4, QByteArray::fromRawData(reinterpret_cast<const char*>(&byteRate), 4));
    qint16 blockAlign = numChannels * 2;
    wavHeader.replace(32, 2, QByteArray::fromRawData(reinterpret_cast<const char*>(&blockAlign), 2));
    qint16 bitsPerSample = 16;
    wavHeader.replace(34, 2, QByteArray::fromRawData(reinterpret_cast<const char*>(&bitsPerSample), 2));

    // data chunk
    wavHeader.replace(36, 4, "data");
    qint32 dataSize = audioData.size();
    wavHeader.replace(40, 4, QByteArray::fromRawData(reinterpret_cast<const char*>(&dataSize), 4));

    // Write to file
    QFile file(outputFilename);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(wavHeader);
        file.write(audioData);
        file.close();
        qInfo() << "💾 Audio saved to:" << outputFilename;
    } else {
        qWarning() << "⚠️  Failed to save audio file:" << outputFilename;
    }
}

void PresentationDemo::displayStatus(const QString &module, const QString &status, const QString &details)
{
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss");
    qInfo() << QString("[%1] %2 %3").arg(timestamp, module, status);
    if (!details.isEmpty()) {
        qInfo() << "    └─" << details;
    }
}

void PresentationDemo::displayResult(const QString &module, const QString &result)
{
    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss");
    qInfo() << QString("[%1] %2 ✅ Result:").arg(timestamp, module);
    qInfo() << "    └─" << QString("\"%1\"").arg(result);
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    // Set application information
    app.setApplicationName("Deepin AI Presentation Demo");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("UnionTech Software Technology Co., Ltd.");

    qInfo() << "Deepin AI Presentation Demo - Version 1.0.0";
    qInfo() << "Comprehensive demonstration of AI capabilities";
    qInfo() << "Organization: UnionTech Software Technology Co., Ltd.";
    qInfo() << "";

    PresentationDemo demo;
    demo.startPresentation();

    return app.exec();
}

#include "presentation_demo.moc"
