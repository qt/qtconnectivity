// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

package org.qtproject.qt.android.bluetooth;

import java.io.InputStream;
import java.io.IOException;
import android.util.Log;

@SuppressWarnings("WeakerAccess")
public class QtBluetoothInputStreamThread extends Thread
{
    /* Pointer to the Qt object that "owns" the Java object */
    @SuppressWarnings("CanBeFinal")
    long qtObject = 0;
    @SuppressWarnings("CanBeFinal")
    public boolean logEnabled = false;
    private static final String TAG = "QtBluetooth";
    private InputStream m_inputStream = null;

    //error codes
    public static final int QT_MISSING_INPUT_STREAM = 0;
    public static final int QT_READ_FAILED = 1;
    public static final int QT_THREAD_INTERRUPTED = 2;

    public QtBluetoothInputStreamThread()
    {
        setName("QtBtInputStreamThread");
    }

    public void setInputStream(InputStream stream)
    {
        m_inputStream = stream;
    }

    public void run()
    {
        if (m_inputStream == null) {
            errorOccurred(qtObject, QT_MISSING_INPUT_STREAM);
            return;
        }

        byte[] buffer = new byte[1000];
        int bytesRead;

        try {
            while (!isInterrupted()) {
                //this blocks until we see incoming data
                //or close() on related BluetoothSocket is called
                bytesRead = m_inputStream.read(buffer);
                readyData(qtObject, buffer, bytesRead);
            }

            errorOccurred(qtObject, QT_THREAD_INTERRUPTED);
        } catch (IOException ex) {
            if (logEnabled)
                Log.d(TAG, "InputStream.read() failed:" + ex.toString());
            ex.printStackTrace();
            errorOccurred(qtObject, QT_READ_FAILED);
        }

        if (logEnabled)
            Log.d(TAG, "Leaving input stream thread");
    }

    public static native void errorOccurred(long qtObject, int errorCode);
    public static native void readyData(long qtObject, byte[] buffer, int bufferLength);
}
