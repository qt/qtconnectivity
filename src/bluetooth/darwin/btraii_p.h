/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
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
******************************************************************************/

#ifndef BTRAII_P_H
#define BTRAII_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtCore/qglobal.h>

#include <utility>

QT_BEGIN_NAMESPACE

namespace DarwinBluetooth {

enum class RetainPolicy
{
    noInitialRetain,
    doInitialRetain
};

// The class StrongReference and its descendant ScopedGuard
// are RAII classes dealing with raw pointers to NSObject class
// and its descendants (and thus hiding Objective-C's retain/
// release semantics). The header itself is meant to be included
// into *.cpp files so it's a pure C++ code without any Objective-C
// syntax. Thus it's a bit clunky - the type information is 'erased'
// and has to be enforced by the code using these smart pointers.
// That's because these types are Objective-C classes - thus require
// Objective-C compiler to work. Member-function template 'getAs' is
// a convenience shortcut giving the desired pointer type in
// Objective-C++ files (*.mm).

// TODO: on top of these classes I can build ObjCStrongReference (it's
// now inside osxbtutils_p.h, a template class that does have type
// information needed but works only in Objective-C++ environment.
class StrongReference
{
public:
    StrongReference() = default;
    StrongReference(void *object, RetainPolicy policy);
    StrongReference(const StrongReference &other);
    StrongReference(StrongReference &&other);

    ~StrongReference();

    StrongReference &operator = (const StrongReference &other) noexcept;
    StrongReference &operator = (StrongReference &&other) noexcept;

    void swap(StrongReference &other) noexcept
    {
        std::swap(objCInstance, other.objCInstance);
    }

    void *release();

    void reset();
    void reset(void *newInstance, RetainPolicy policy);

    template<class ObjCType>
    ObjCType *getAs() const
    {
        return static_cast<ObjCType *>(objCInstance);
    }

    operator bool() const
    {
        return !!objCInstance;
    }

private:
    void *objCInstance = nullptr;
};

class ScopedPointer final : public StrongReference
{
public:
    ScopedPointer() = default;
    ScopedPointer(void *instance, RetainPolicy policy)
        : StrongReference(instance, policy)
    {
    }

private:
    Q_DISABLE_COPY_MOVE(ScopedPointer)
};

} // namespace DarwinBluetooth

QT_END_NAMESPACE

#endif // BTRAII_P_H
