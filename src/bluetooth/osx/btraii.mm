/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtBluetooth module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
****************************************************************************/

#include "btraii_p.h"

#include <qdebug.h>

#include <Foundation/Foundation.h>

#include <utility>

QT_BEGIN_NAMESPACE

namespace DarwinBluetooth {

StrongReference::StrongReference(void *object, RetainPolicy policy)
    : objCInstance(object)
{
    if (policy == RetainPolicy::doInitialRetain)
        objCInstance = [getAs<NSObject>() retain];
}

StrongReference::StrongReference(const StrongReference &other)
{
    objCInstance = [other.getAs<NSObject>() retain];
}

StrongReference::StrongReference(StrongReference &&other)
{
    std::swap(objCInstance, other.objCInstance);
}

StrongReference::~StrongReference()
{
    [getAs<NSObject>() release];
}

StrongReference &StrongReference::operator = (const StrongReference &other) noexcept
{
    if (this != &other) {
        [getAs<NSObject>() release];
        objCInstance = [other.getAs<NSObject>() retain];
    }

    return *this;
}

StrongReference &StrongReference::operator = (StrongReference &&other) noexcept
{
    swap(other);
    return *this;
}

void StrongReference::reset()
{
    [getAs<NSObject>() release];
    objCInstance = nullptr;
}

void StrongReference::reset(void *obj, RetainPolicy policy)
{
    [getAs<NSObject>() release];
    objCInstance = obj;

    if (policy == RetainPolicy::doInitialRetain) {
        auto newInstance = static_cast<NSObject *>(obj);
        Q_ASSERT(newInstance);
        objCInstance = [newInstance retain];
    }
}

} // namespace DarwinBluetooth

QT_END_NAMESPACE
