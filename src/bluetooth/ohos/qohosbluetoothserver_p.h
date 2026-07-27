// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QOHOSBLUETOOTHSERVER_P_H
#define QOHOSBLUETOOTHSERVER_P_H

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

#include <QtBluetooth/private/qohosbluetoothcommon_p.h>

#include <QtCore/qobject.h>
#include <QtCore/qstring.h>
#include <QtCore/qtconfigmacros.h>

#include <cstdint>
#include <memory>
#include <optional>

QT_BEGIN_NAMESPACE

namespace QtOhosBluetooth {

class QOhosBluetoothServerProxy
    : public QObject, public std::enable_shared_from_this<QOhosBluetoothServerProxy>
{
    Q_OBJECT
public:
    QOhosBluetoothServerProxy();

    bool listen(const QString &serviceName, const QString &uuid, bool secure);
    bool accept();
    void close();
    bool isServiceRegistered() const;

Q_SIGNALS:
    void clientAccepted(int clientSocketDescriptor);
    void acceptingStopped();
    void errorOccurred(QOhosBluetoothErrorCode errorCode);

private:
    void onClientAccepted(std::shared_ptr<int> clientSocketHandle);
    void onAcceptFailed(QOhosBluetoothErrorCode errorCode);
    void acceptNextConnection();
    void scheduleAcceptNextConnection();
    void reportError(QOhosBluetoothErrorCode errorCode);
    void reportAcceptingStopped();

    std::shared_ptr<int> m_serverSocketHandle;
    std::optional<QString> m_registeredServiceName;
};

}

QT_END_NAMESPACE

#endif // QOHOSBLUETOOTHSERVER_P_H
