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
 ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** **********************************************************************/

package org.qtproject.qt.android.bluetooth;

import android.bluetooth.BluetoothGattCharacteristic;
import android.util.Log;

import java.util.UUID;

public class QtBluetoothGattCharacteristic extends BluetoothGattCharacteristic {
    public QtBluetoothGattCharacteristic(UUID uuid, int properties, int permissions,
                                         int minimumValueLength, int maximumValueLength) {
        super(uuid, properties, permissions);
        minValueLength = minimumValueLength;
        maxValueLength = maximumValueLength;
    }
    public int minValueLength;
    public int maxValueLength;
}
