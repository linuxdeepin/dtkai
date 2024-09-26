// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DCHATCOMPLETIONS_P_H
#define DCHATCOMPLETIONS_P_H

#include "dchatcompletions.h"
#include "aidaemon_apisession_chat.h"

DAI_BEGIN_NAMESPACE

class DChatCompletionsPrivate : public QObject
{
    Q_OBJECT
public:
    explicit DChatCompletionsPrivate(DChatCompletions *q);
    ~DChatCompletionsPrivate();
    bool ensureServer();
    static QString packageParams(const QList<ChatHistory> &history, const QVariantHash &params);
public Q_SLOTS:
    void finished(int error, const QString &content);
public:
    QMutex mtx;
    bool running = false;
    DTK_CORE_NAMESPACE::DError error;
    QScopedPointer<OrgDeepinAiDaemonAPISessionChatInterface> chatIfs;
public:
    DChatCompletions *q = nullptr;
};

DAI_END_NAMESPACE

#endif // DCHATCOMPLETIONS_P_H
