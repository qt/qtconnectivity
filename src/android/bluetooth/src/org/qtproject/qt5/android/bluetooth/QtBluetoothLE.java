/****************************************************************************
 **
 ** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
 ** Contact: http://www.qt-project.org/legal
 **
 ** This file is part of the QtBluetooth module of the Qt Toolkit.
 **
 ** $QT_BEGIN_LICENSE:LGPL21$
 ** Commercial License Usage
 ** Licensees holding valid commercial Qt licenses may use this file in
 ** accordance with the commercial license agreement provided with the
 ** Software or, alternatively, in accordance with the terms contained in
 ** a written agreement between you and Digia. For licensing terms and
 ** conditions see http://qt.digia.com/licensing. For further information
 ** use the contact form at http://qt.digia.com/contact-us.
 **
 ** GNU Lesser General Public License Usage
 ** Alternatively, this file may be used under the terms of the GNU Lesser
 ** General Public License version 2.1 or version 3 as published by the Free
 ** Software Foundation and appearing in the file LICENSE.LGPLv21 and
 ** LICENSE.LGPLv3 included in the packaging of this file. Please review the
 ** following information to ensure the GNU Lesser General Public License
 ** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
 ** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
 **
 ** In addition, as a special exception, Digia gives you certain additional
 ** rights. These rights are described in the Digia Qt LGPL Exception
 ** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
 **
 ** $QT_END_LICENSE$
 **
 ****************************************************************************/

package org.qtproject.qt5.android.bluetooth;

import android.app.Activity;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCallback;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattDescriptor;
import android.bluetooth.BluetoothGattService;
import android.bluetooth.BluetoothProfile;
import android.util.Log;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Hashtable;
import java.util.List;
import java.util.UUID;

public class QtBluetoothLE {
    private static final String TAG = "QtBluetoothGatt";
    private BluetoothAdapter mBluetoothAdapter;
    private boolean mLeScanRunning = false;

    private BluetoothGatt mBluetoothGatt = null;
    private String mRemoteGattAddress;
    private BluetoothDevice mRemoteGattDevice = null;


    /* Pointer to the Qt object that "owns" the Java object */
    long qtObject = 0;
    Activity qtactivity = null;

    public QtBluetoothLE() {
        mBluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
    }

    public QtBluetoothLE(final String remoteAddress, Activity activity) {
        this();
        qtactivity = activity;
        mRemoteGattAddress = remoteAddress;
    }


    /*************************************************************/
    /* Device scan                                               */
    /*************************************************************/

    /*
        Returns true, if request was successfully completed
     */
    public boolean scanForLeDevice(final boolean isEnabled) {
        if (isEnabled == mLeScanRunning)
            return true;

        if (isEnabled) {
            mLeScanRunning = mBluetoothAdapter.startLeScan(leScanCallback);
        } else {
            mBluetoothAdapter.stopLeScan(leScanCallback);
            mLeScanRunning = false;
        }

        return (mLeScanRunning == isEnabled);
    }

    // Device scan callback
    private BluetoothAdapter.LeScanCallback leScanCallback =
            new BluetoothAdapter.LeScanCallback() {

                @Override
                public void onLeScan(final BluetoothDevice device, int rssi, byte[] scanRecord) {
                    if (qtObject == 0)
                        return;

                    leScanResult(qtObject, device, rssi);
                }
            };

    public native void leScanResult(long qtObject, BluetoothDevice device, int rssi);

    /*************************************************************/
    /* Service Discovery                                         */
    /*************************************************************/

    private BluetoothGattCallback gattCallback = new BluetoothGattCallback() {

        public void onConnectionStateChange(BluetoothGatt gatt, int status, int newState) {
            if (qtObject == 0)
                return;

            int qLowEnergyController_State = 0;
            //This must be in sync with QLowEnergyController::ControllerState
            switch (newState) {
                case BluetoothProfile.STATE_DISCONNECTED:
                    qLowEnergyController_State = 0; break;
                case BluetoothProfile.STATE_CONNECTED:
                    qLowEnergyController_State = 2;
            }

            //This must be in sync with QLowEnergyController::Error
            int errorCode;
            switch (status) {
                case BluetoothGatt.GATT_SUCCESS:
                    errorCode = 0; break; //QLowEnergyController::NoError
                default:
                    Log.w(TAG, "Unhandled error code on connectionStateChanged: " + status);
                    errorCode = status; break; //TODO deal with all errors
            }
            leConnectionStateChange(qtObject, errorCode, qLowEnergyController_State);
        }

        public void onServicesDiscovered(BluetoothGatt gatt, int status) {
            //This must be in sync with QLowEnergyController::Error
            int errorCode;
            StringBuilder builder = new StringBuilder();
            switch (status) {
                case BluetoothGatt.GATT_SUCCESS:
                    errorCode = 0; //QLowEnergyController::NoError
                    final List<BluetoothGattService> services = mBluetoothGatt.getServices();
                    for (BluetoothGattService service: services) {
                        builder.append(service.getUuid().toString() + " "); //space is separator
                    }
                    break;
                default:
                    Log.w(TAG, "Unhandled error code on onServicesDiscovered: " + status);
                    errorCode = status; break; //TODO deal with all errors
            }
            leServicesDiscovered(qtObject, errorCode, builder.toString());
        }

        public void onCharacteristicRead(android.bluetooth.BluetoothGatt gatt,
                                         android.bluetooth.BluetoothGattCharacteristic characteristic,
                                         int status)
        {
            GattEntry entry = entries.get(runningHandle);
            entry.valueKnown = true;
            entries.set(runningHandle, entry);
            performServiceDiscoveryForHandle(runningHandle+1, false);
        }

        public void onCharacteristicWrite(android.bluetooth.BluetoothGatt gatt,
                                          android.bluetooth.BluetoothGattCharacteristic characteristic,
                                          int status)
        {
            System.out.println("onCharacteristicWrite");
        }

        public void onCharacteristicChanged(android.bluetooth.BluetoothGatt gatt,
                                            android.bluetooth.BluetoothGattCharacteristic characteristic)
        {
            System.out.println("onCharacteristicChanged");
        }

        public void onDescriptorRead(android.bluetooth.BluetoothGatt gatt,
                                     android.bluetooth.BluetoothGattDescriptor descriptor,
                                     int status)
        {
            GattEntry entry = entries.get(runningHandle);
            entry.valueKnown = true;
            entries.set(runningHandle, entry);
            performServiceDiscoveryForHandle(runningHandle+1, false);
        }

        public void onDescriptorWrite(android.bluetooth.BluetoothGatt gatt,
                                      android.bluetooth.BluetoothGattDescriptor descriptor,
                                      int status)
        {
            System.out.println("onDescriptorWrite");
        }
        //TODO Requires Android API 21 which is not available on CI yet.
//        public void onReliableWriteCompleted(android.bluetooth.BluetoothGatt gatt,
//                                             int status) {
//            System.out.println("onReliableWriteCompleted");
//        }
//
//        public void onReadRemoteRssi(android.bluetooth.BluetoothGatt gatt,
//                                     int rssi, int status) {
//            System.out.println("onReadRemoteRssi");
//        }

    };


    public boolean connect() {
        if (mBluetoothGatt != null)
            return mBluetoothGatt.connect();

        mRemoteGattDevice = mBluetoothAdapter.getRemoteDevice(mRemoteGattAddress);
        if (mRemoteGattDevice == null)
            return false;

        mBluetoothGatt = mRemoteGattDevice.connectGatt(qtactivity, false, gattCallback);
        if (mBluetoothGatt == null)
            return false;

        return true;
    }

    public void disconnect() {
        if (mBluetoothGatt == null)
            return;

        mBluetoothGatt.disconnect();
    }

    public boolean discoverServices()
    {
        if (mBluetoothGatt == null)
            return false;

        return mBluetoothGatt.discoverServices();
    }

    private enum GattEntryType
    {
        Service, Characteristic, CharacteristicValue, Descriptor
    }
    private class GattEntry
    {
        public GattEntryType type;
        public boolean valueKnown = false;
        public BluetoothGattService service = null;
        public BluetoothGattCharacteristic characteristic = null;
        public BluetoothGattDescriptor descriptor = null;
    }
    Hashtable<UUID, List<Integer>> uuidToEntry = new Hashtable<UUID, List<Integer>>(100);
    ArrayList<GattEntry> entries = new ArrayList<GattEntry>(100);

    private void populateHandles()
    {
        // We introduce the notion of artificial handles. While GATT handles
        // are not exposed on Android they help to quickly identify GATT attributes
        // on the C++ side. The Qt Api will not expose the handles
        GattEntry entry = null;
        List<BluetoothGattService> services = mBluetoothGatt.getServices();
        for (BluetoothGattService service: services) {
            entry = new GattEntry();
            entry.type = GattEntryType.Service;
            entry.service = service;
            entries.add(entry);

            //some devices may have more than one service with the same uuid
            List<Integer> old = uuidToEntry.get(service.getUuid());
            if (old == null)
                old = new ArrayList<Integer>();
            old.add(entries.size()-1);
            uuidToEntry.put(service.getUuid(), old);

            List<BluetoothGattCharacteristic> charList = service.getCharacteristics();
            for (BluetoothGattCharacteristic characteristic: charList) {
                entry = new GattEntry();
                entry.type = GattEntryType.Characteristic;
                entry.characteristic = characteristic;
                entries.add(entry);
                //uuidToEntry.put(characteristic.getUuid(), entries.size()-1);

                // this emulates GATT value attributes
                entry = new GattEntry();
                entry.type = GattEntryType.CharacteristicValue;
                entries.add(entry);
                //uuidToEntry.put(characteristic.getUuid(), entry);

                List<BluetoothGattDescriptor> descList = characteristic.getDescriptors();
                for (BluetoothGattDescriptor desc: descList) {
                    entry = new GattEntry();
                    entry.type = GattEntryType.Descriptor;
                    entry.descriptor = desc;
                    entries.add(entry);
                    //uuidToEntry.put(desc.getUuid(), entries.size()-1);
                }
            }
        }

        entries.trimToSize();
    }

    private int currentServiceInDiscovery = -1;
    private int runningHandle = -1;
    public synchronized boolean discoverServiceDetails(String serviceUuid)
    {
        try {
            if (mBluetoothGatt == null)
                return false;

            if (entries.isEmpty())
                populateHandles();

            GattEntry entry;
            int serviceHandle;
            try {
                UUID service = UUID.fromString(serviceUuid);
                List<Integer> handles = uuidToEntry.get(service);
                if (handles == null || handles.isEmpty()) {
                    Log.w(TAG, "Unknown service uuid for current device: " + service.toString());
                    return false;
                }

                //TODO for now we assume we always want the first service in case of uuid collision
                serviceHandle = handles.get(0);
                entry = entries.get(serviceHandle);
                if (entry == null) {
                    Log.w(TAG, "Service with UUID " + service.toString() + " not found");
                    return false;
                }
            } catch (IllegalArgumentException ex) {
                //invalid UUID string passed
                Log.w(TAG, "Cannot parse given UUID");
                return false;
            }

            if (entry.type != GattEntryType.Service) {
                Log.w(TAG, "Given UUID is not a service UUID: " + serviceUuid);
                return false;
            }

            // current service already under investigation
            if (currentServiceInDiscovery == serviceHandle)
                return true;

            if (currentServiceInDiscovery != -1) {
                Log.w(TAG, "Service discovery already running on another service");
                return false;
            }

            if (!entry.valueKnown) {
                performServiceDiscoveryForHandle(serviceHandle, true);
            } else {
                Log.w(TAG, "Service already discovered");
            }

        } catch (Exception ex) {
            ex.printStackTrace();
            return false;
        }

        return true;
    }

    private void finishCurrentServiceDiscovery()
    {
        GattEntry discoveredService = entries.get(currentServiceInDiscovery);
        discoveredService.valueKnown = true;
        entries.set(currentServiceInDiscovery, discoveredService);
        runningHandle = -1;
        currentServiceInDiscovery = -1;
        leServiceDetailDiscoveryFinished(qtObject, discoveredService.service.getUuid().toString());
    }

    private synchronized void performServiceDiscoveryForHandle(int nextHandle, boolean searchStarted)
    {
        try {
            if (searchStarted) {
                currentServiceInDiscovery = nextHandle;
                runningHandle = ++nextHandle;
            } else {
                runningHandle = nextHandle;
            }

            GattEntry entry = null;
            try {
                entry = entries.get(nextHandle);
            } catch (IndexOutOfBoundsException ex) {
                ex.printStackTrace();
                Log.w(TAG, "Last entry of last service read");
                finishCurrentServiceDiscovery();
                return;
            }

            boolean result = false;
            switch (entry.type) {
                case Characteristic:
                    result = mBluetoothGatt.readCharacteristic(entry.characteristic);
                    if (!result)
                        performServiceDiscoveryForHandle(runningHandle+1, false);
                    break;
                case CharacteristicValue:
                    // ignore -> nothing to do for this artificial type
                    performServiceDiscoveryForHandle(runningHandle+1, false);
                    break;
                case Descriptor:
                    result = mBluetoothGatt.readDescriptor(entry.descriptor);
                    if (!result)
                        performServiceDiscoveryForHandle(runningHandle+1, false);
                    break;
                case Service:
                    finishCurrentServiceDiscovery();
                    break;
                default:
                    Log.w(TAG, "Invalid GATT attribute type");
                    break;
            }

        } catch(Exception ex) {
            ex.printStackTrace();
        }
    }

    public native void leConnectionStateChange(long qtObject, int wasErrorTransition, int newState);
    public native void leServicesDiscovered(long qtObject, int errorCode, String uuidList);
    public native void leServiceDetailDiscoveryFinished(long qtObject, final String serviceUuid);
}

