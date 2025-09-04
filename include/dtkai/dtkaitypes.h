// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DTKAITYPES_H
#define DTKAITYPES_H

#include "dtkai_global.h"

#include <QString>

DAI_BEGIN_NAMESPACE

struct ChatHistory
{
    QString role;
    QString content;
};

inline constexpr char kChatRoleUser[] { "user" };
inline constexpr char kChatRoleAssistant[] { "assistant" };
inline constexpr char kChatRoleSystem[] { "system" };

DAI_END_NAMESPACE

#endif // DTKAITYPES_H
