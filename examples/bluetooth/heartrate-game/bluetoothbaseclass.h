// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef BLUETOOTHBASECLASS_H
#define BLUETOOTHBASECLASS_H

#include <QtCore/qobject.h>

class BluetoothBaseClass : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString error READ error WRITE setError NOTIFY errorChanged)
    Q_PROPERTY(QString info READ info WRITE setInfo NOTIFY infoChanged)

public:
    explicit BluetoothBaseClass(QObject *parent = nullptr);

    QString error() const;
    void setError(const QString& error);

    QString info() const;
    void setInfo(const QString& info);

    void clearMessages();

signals:
    void errorChanged();
    void infoChanged();

private:
    QString m_error;
    QString m_info;
};

#endif // BLUETOOTHBASECLASS_H
