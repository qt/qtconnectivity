// Copyright (C) 2018 Governikus GmbH & Co. KG
// SPDX-License-Identifier: LicenseRef-Qt-Commercial

package org.qtproject.qt.android.nfc;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.nfc.NfcAdapter;

class QtNfcBroadcastReceiver extends BroadcastReceiver
{
    final private long qtObject;
    final private Context qtContext;

    QtNfcBroadcastReceiver(long obj, Context context)
    {
        qtObject = obj;
        qtContext = context;
        IntentFilter filter = new IntentFilter(NfcAdapter.ACTION_ADAPTER_STATE_CHANGED);
        qtContext.registerReceiver(this, filter);
    }

    void unregisterReceiver()
    {
        qtContext.unregisterReceiver(this);
    }

    @Override
    public void onReceive(Context context, Intent intent)
    {
        final int state = intent.getIntExtra(NfcAdapter.EXTRA_ADAPTER_STATE, NfcAdapter.STATE_OFF);
        jniOnReceive(qtObject, state);
    }

    native void jniOnReceive(long qtObject, int state);
}
