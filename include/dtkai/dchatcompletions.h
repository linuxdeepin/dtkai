// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DCHATCOMPLETIONS_H
#define DCHATCOMPLETIONS_H

#include <QObject>
#include <QScopedPointer>

#include <DError>

#include "dtkai_global.h"
#include "dtkaitypes.h"

DAI_BEGIN_NAMESPACE
class DChatCompletionsPrivate;
class DChatCompletions : public QObject
{
    Q_OBJECT
    friend class DChatCompletionsPrivate;
public:
    explicit DChatCompletions(QObject *parent = nullptr);
    ~DChatCompletions();
    bool chatStream(const QString &prompt, const QList<ChatHistory> &history = {}, const QVariantHash &params = {});
    QString chat(const QString &prompt, const QList<ChatHistory> &history = {}, const QVariantHash &params = {});
    void terminate();
    DTK_CORE_NAMESPACE::DError lastError() const;
Q_SIGNALS:
    void streamOutput(const QString &content);
    void streamFinished(int error);
private:
    QScopedPointer<DChatCompletionsPrivate> d;
};

DAI_END_NAMESPACE

#endif // DCHATCOMPLETIONS_H
