/***************************************************************************
**
** Copyright (C) 2013 Centria research and development
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtNfc module of the Qt Toolkit.
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

package org.qtproject.qt5.android.nfc;

import java.lang.Thread;
import java.lang.Runnable;

import android.os.Parcelable;
import android.os.Looper;
import android.content.Context;
import android.app.Activity;
import android.app.PendingIntent;
import android.content.Intent;
import android.content.IntentFilter;
import android.nfc.NfcAdapter;
import android.content.IntentFilter.MalformedMimeTypeException;
import android.os.Bundle;
//import android.util.Log;
import android.content.BroadcastReceiver;

public class QtNfc
{
    /* static final QtNfc m_nfc = new QtNfc(); */
    static private final String TAG = "QtNfc";
    static public NfcAdapter m_adapter = null;
    static public PendingIntent m_pendingIntent = null;
    static public IntentFilter[] m_filters;
    static public Activity m_activity;

    static public void setActivity(Activity activity, Object activityDelegate)
    {
        //Log.d(TAG, "setActivity " + activity);
        m_activity = activity;
        m_adapter = NfcAdapter.getDefaultAdapter(m_activity);
        if (m_adapter == null) {
            //Log.e(TAG, "No NFC available");
            return;
        }
        m_pendingIntent = PendingIntent.getActivity(
            m_activity,
            0,
            new Intent(m_activity, m_activity.getClass()).addFlags(Intent.FLAG_ACTIVITY_SINGLE_TOP),
            0);

        //Log.d(TAG, "Pending intent:" + m_pendingIntent);

        IntentFilter filter = new IntentFilter(NfcAdapter.ACTION_NDEF_DISCOVERED);

        m_filters = new IntentFilter[]{
            filter
        };

        try {
            filter.addDataType("*/*");
        } catch(MalformedMimeTypeException e) {
            throw new RuntimeException("Fail", e);
        }

        //Log.d(TAG, "Thread:" + Thread.currentThread().getId());
    }

    static public void start()
    {
        if (m_adapter == null) return;
        m_activity.runOnUiThread(new Runnable() {
            public void run() {
                //Log.d(TAG, "Enabling NFC");
                m_adapter.enableForegroundDispatch(m_activity, m_pendingIntent, null, null);
            }
        });
    }

    static public void stop()
    {
        if (m_adapter == null) return;
        m_activity.runOnUiThread(new Runnable() {
            public void run() {
                //Log.d(TAG, "Disabling NFC");
                m_adapter.disableForegroundDispatch(m_activity);
            }
        });
    }

    static public boolean isAvailable()
    {
        m_adapter = NfcAdapter.getDefaultAdapter(m_activity);
        if (m_adapter == null) {
            //Log.e(TAG, "No NFC available (Adapter is null)");
            return false;
        }
        return m_adapter.isEnabled();
    }
}
