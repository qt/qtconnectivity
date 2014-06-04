/***************************************************************************
**
** Copyright (C) 2013 BlackBerry Limited all rights reserved
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtBluetooth module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtTest/QtTest>
#include <QUuid>

#include <QDebug>

#include <qbluetoothdeviceinfo.h>
#include <qlowenergydescriptorinfo.h>
#include <qbluetoothlocaldevice.h>
#include "qbluetoothuuid.h"

QT_USE_NAMESPACE

Q_DECLARE_METATYPE(QUuid)
Q_DECLARE_METATYPE(QBluetoothUuid::DescriptorType)

class tst_QLowEnergyDescriptorInfo : public QObject
{
    Q_OBJECT

public:
    tst_QLowEnergyDescriptorInfo();
    ~tst_QLowEnergyDescriptorInfo();

private slots:
    void initTestCase();
    void tst_construction();
    void tst_assignment_data();
    void tst_assignment();
};

tst_QLowEnergyDescriptorInfo::tst_QLowEnergyDescriptorInfo()
{
}

tst_QLowEnergyDescriptorInfo::~tst_QLowEnergyDescriptorInfo()
{
}

void tst_QLowEnergyDescriptorInfo::initTestCase()
{
    // start Bluetooth if not started
    QBluetoothLocalDevice *device = new QBluetoothLocalDevice();
    device->powerOn();
    delete device;
}

void tst_QLowEnergyDescriptorInfo::tst_construction()
{
    const QBluetoothUuid descriptorUuid(QString("0x2902").toUShort(0,0));
    const QBluetoothUuid alternateDescriptorUuid(QString("0x2906").toUShort(0,0));

    {
        QLowEnergyDescriptorInfo descriptorInfo = QLowEnergyDescriptorInfo(QBluetoothUuid());

        QCOMPARE(descriptorInfo.uuid(), QBluetoothUuid());
        QCOMPARE(descriptorInfo.value(), QByteArray());
        QCOMPARE(descriptorInfo.handle(), QString("0x0000"));
        QCOMPARE(descriptorInfo.name(), QString(""));
    }

    {
        QLowEnergyDescriptorInfo descriptorInfo(descriptorUuid);

        QCOMPARE(descriptorInfo.uuid().toString(), descriptorUuid.toString());

        QLowEnergyDescriptorInfo copyInfo = descriptorInfo;
        QCOMPARE(copyInfo.uuid().toString(), descriptorUuid.toString());
        QCOMPARE(copyInfo.name(), QString(""));
        copyInfo.setValue(QByteArray("test"));
        QCOMPARE(copyInfo.value(), QByteArray("test"));

        //QLowEnergyDescriptorInfos share their internal data
        //for now enshrine this in the test
        //TODO do we really need this behavior as it is unusual for value types
        QCOMPARE(descriptorInfo.value(), QByteArray("test"));

        copyInfo = QLowEnergyDescriptorInfo(alternateDescriptorUuid);
        QCOMPARE(copyInfo.uuid().toString(), alternateDescriptorUuid.toString());

        QCOMPARE(copyInfo.uuid(), alternateDescriptorUuid);
        QCOMPARE(copyInfo.value(), QByteArray());
        QCOMPARE(copyInfo.handle(), QString("0x0000"));
        QCOMPARE(copyInfo.name(), QString(""));
    }
}

void tst_QLowEnergyDescriptorInfo::tst_assignment_data()
{
    QTest::addColumn<QBluetoothUuid>("descriptorClassUuid");

    QTest::newRow("0x000000 COD") << QBluetoothUuid(QString("0x2901").toUShort(0,0));
    QTest::newRow("0x001000 COD") << QBluetoothUuid(QString("0x2902").toUShort(0,0));
    QTest::newRow("0x002000 COD") << QBluetoothUuid(QString("0x2903").toUShort(0,0));
    QTest::newRow("0x003000 COD") << QBluetoothUuid(QString("0x2904").toUShort(0,0));
    QTest::newRow("0x004000 COD") << QBluetoothUuid(QString("0x2905").toUShort(0,0));
    QTest::newRow("0x005000 COD") << QBluetoothUuid(QString("0x2906").toUShort(0,0));
    QTest::newRow("0x006000 COD") << QBluetoothUuid(QString("0x2907").toUShort(0,0));
    QTest::newRow("0x007000 COD") << QBluetoothUuid(QString("0x2908").toUShort(0,0));
    QTest::newRow("0x008000 COD") << QBluetoothUuid(QString("0x2900").toUShort(0,0));
}

void tst_QLowEnergyDescriptorInfo::tst_assignment()
{
    QFETCH(QBluetoothUuid, descriptorClassUuid);

    QLowEnergyDescriptorInfo descriptorInfo(descriptorClassUuid);


    {
        QLowEnergyDescriptorInfo copyInfo = descriptorInfo;

        QCOMPARE(copyInfo.uuid(), descriptorClassUuid);
        QCOMPARE(copyInfo.value(), QByteArray());
    }

    {
        QLowEnergyDescriptorInfo copyInfo = QLowEnergyDescriptorInfo(QBluetoothUuid());

        copyInfo = descriptorInfo;

        QCOMPARE(copyInfo.uuid(), descriptorClassUuid);
    }

    {
        QLowEnergyDescriptorInfo copyInfo1 = QLowEnergyDescriptorInfo(QBluetoothUuid());
        QLowEnergyDescriptorInfo copyInfo2 = QLowEnergyDescriptorInfo(QBluetoothUuid());

        copyInfo1 = copyInfo2 = descriptorInfo;

        QCOMPARE(copyInfo1.uuid(), descriptorClassUuid);
        QCOMPARE(copyInfo2.uuid(), descriptorClassUuid);
    }

}

QTEST_MAIN(tst_QLowEnergyDescriptorInfo)

#include "tst_qlowenergydescriptorinfo.moc"

