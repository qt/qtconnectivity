// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

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
