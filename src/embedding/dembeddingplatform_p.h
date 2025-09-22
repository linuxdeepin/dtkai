// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DEMBEDDINGPLATFORM_P_H
#define DEMBEDDINGPLATFORM_P_H

#include "dembeddingplatform.h"
#include <DObjectPrivate>

DAI_BEGIN_NAMESPACE

class DEmbeddingPlatformPrivate : public Dtk::Core::DObjectPrivate
{
    Q_DECLARE_PUBLIC(DEmbeddingPlatform)

public:
    explicit DEmbeddingPlatformPrivate(DEmbeddingPlatform *qq);
    
    DTK_CORE_NAMESPACE::DError error;
};

DAI_END_NAMESPACE

#endif // DEMBEDDINGPLATFORM_P_H