// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFUNCTIONCALLING_P_H
#define DFUNCTIONCALLING_P_H

#include "dfunctioncalling.h"
#include "aidaemon_apisession_functioncalling.h"

DAI_BEGIN_NAMESPACE

class DFunctionCallingPrivate : public QObject
{
    Q_OBJECT
public:
    explicit DFunctionCallingPrivate(DFunctionCalling *q);
    ~DFunctionCallingPrivate();
    bool ensureServer();
    static QString packageParams(const QVariantHash &params);
public:
    QMutex mtx;
    bool running = false;
    DTK_CORE_NAMESPACE::DError error;
    QScopedPointer<OrgDeepinAiDaemonAPISessionFunctionCallingInterface> funcIfs;
public:
    DFunctionCalling *q = nullptr;
};

DAI_END_NAMESPACE
#endif // DFUNCTIONCALLING_P_H
