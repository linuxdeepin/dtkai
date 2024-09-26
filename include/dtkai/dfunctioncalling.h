// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFUNCTIONCALLING_H
#define DFUNCTIONCALLING_H

#include <QObject>
#include <QScopedPointer>

#include <DError>

#include "dtkai_global.h"
#include "dtkaitypes.h"

DAI_BEGIN_NAMESPACE

class DFunctionCallingPrivate;
class DFunctionCalling : public QObject
{
    Q_OBJECT
    friend class DFunctionCallingPrivate;
public:
    explicit DFunctionCalling(QObject *parent = nullptr);
    ~DFunctionCalling();
    QString parse(const QString &prompt, const QString &functions, const QVariantHash &params = {});
    void terminate();
    DTK_CORE_NAMESPACE::DError lastError() const;
private:
    QScopedPointer<DFunctionCallingPrivate> d;
};

DAI_END_NAMESPACE

#endif // DFUNCTIONCALLING_H
