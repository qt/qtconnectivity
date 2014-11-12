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
import java.util.Hashtable;
import java.util.LinkedList;
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
                    qLowEnergyController_State = 0;
                    // we disconnected -> get rid of data from previous run
                    resetServiceDetailData();
                    // reset mBluetoothGatt, reusing same object is not very reliable
                    // sometimes it reconnects and sometimes it does not.
                    mBluetoothGatt = null;
                    break;
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
            if (status != BluetoothGatt.GATT_SUCCESS) {
                Log.w(TAG, "onCharacteristicRead error: " + status);
                return;
            }

            synchronized (this) {
                if (uuidToEntry.isEmpty()) // ignore data if internal setup is not ready;
                    return;
            }

            //runningHandle is only used during serviceDetailsDiscovery
            //If it is -1 we got an update outside of the details discovery process
            if (runningHandle == -1) {
                List<Integer> handles = uuidToEntry.get(characteristic.getService().getUuid());
                if (handles == null || handles.isEmpty()) {
                    Log.w(TAG, "Received Characteristic read update for unknown characteristic");
                    return;
                }
                int serviceHandle = handles.get(0);
                GattEntry entry;
                int foundHandle = -1;
                try {
                    for (int i = 1; serviceHandle + i < entries.size() && foundHandle == -1; i++) {
                        entry = entries.get(serviceHandle + i);
                        if (entry == null)
                            continue;

                        if (entry.type == GattEntryType.Service) {
                            Log.w(TAG, "Out-of-detail-discovery: found unknown characteristic for known service");
                            break; //reached next service -> unknown characteristic in service
                        }

                        if (entry.type != GattEntryType.Characteristic)
                            continue;

                        if (entry.characteristic == characteristic)
                            foundHandle = serviceHandle + i;
                    }
                } catch (IndexOutOfBoundsException ex) {
                    Log.w(TAG, "Out-of-detail-discovery: cannot find handle for characteristic");
                    return;
                }

                if (foundHandle == -1) {
                    Log.w(TAG, "Out-of-detail-discovery: char update failed");
                    return;
                }

                leCharacteristicRead(qtObject, characteristic.getService().getUuid().toString(),
                        foundHandle+1, characteristic.getUuid().toString(),
                        characteristic.getProperties(), characteristic.getValue());

                return;
            }

            GattEntry entry = entries.get(runningHandle);
            entry.valueKnown = true;
            entries.set(runningHandle, entry);

            // Qt manages handles starting at 1, in Java we use a system starting with 0
            //TODO avoid sending service uuid -> service handle should be sufficient
            leCharacteristicRead(qtObject, characteristic.getService().getUuid().toString(),
                    runningHandle + 1, characteristic.getUuid().toString(),
                    characteristic.getProperties(), characteristic.getValue());
            performServiceDetailDiscoveryForHandle(runningHandle + 1, false);
        }

        public void onCharacteristicWrite(android.bluetooth.BluetoothGatt gatt,
                                          android.bluetooth.BluetoothGattCharacteristic characteristic,
                                          int status)
        {
            if (status != BluetoothGatt.GATT_SUCCESS)
                Log.w(TAG, "onCharacteristicWrite: error " + status);

            int handle = handleForCharacteristic(characteristic);
            if (handle == -1) {
                Log.w(TAG,"onCharacteristicWrite: cannot find handle");
                return;
            }

            int errorCode = 0;
            //This must be in sync with QLowEnergyService::ServiceError
            switch (status) {
                case BluetoothGatt.GATT_SUCCESS:
                    errorCode = 0; break; // NoError
                default:
                    errorCode = 2; break; // CharacteristicWriteError
            }

            leCharacteristicWritten(qtObject, handle+1, characteristic.getValue(), errorCode);
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
            if (status != BluetoothGatt.GATT_SUCCESS) {
                Log.w(TAG, "onDescriptorRead error: " + status);
                return;
            }

            synchronized (this) {
                if (uuidToEntry.isEmpty()) // ignore data if internal setup is not ready;
                    return;
            }

            //runningHandle is only used during serviceDetailsDiscovery
            //If it is -1 we got an update outside of the details discovery process
            if (runningHandle == -1) {
                List<Integer> handles = uuidToEntry.get(descriptor.getCharacteristic().getService().getUuid());
                if (handles == null || handles.isEmpty()) {
                    Log.w(TAG, "Received Descriptor read update for unknown descriptor");
                    return;
                }

                int serviceHandle = handles.get(0);
                GattEntry entry;
                int foundHandle = -1;
                try {
                    for (int i = 1; serviceHandle + i < entries.size() && foundHandle == -1; i++) {
                        entry = entries.get(serviceHandle + i);
                        if (entry == null)
                            continue;

                        if (entry.type == GattEntryType.Service) {
                            Log.w(TAG, "Out-of-detail-discovery: found unknown descriptor for known service");
                            break; //reached next service -> unknown descriptor in service
                        }

                        if (entry.type != GattEntryType.Descriptor)
                            continue;

                        if (entry.descriptor == descriptor)
                            foundHandle = serviceHandle + i;
                    }
                } catch (IndexOutOfBoundsException ex) {
                    Log.w(TAG, "Out-of-detail-discovery: cannot find handle for descriptor");
                    return;
                }

                if (foundHandle == -1)
                    Log.w(TAG, "Out-of-detail-discovery: char update failed");

                leDescriptorRead(qtObject, descriptor.getCharacteristic().getService().getUuid().toString(),
                        descriptor.getCharacteristic().getUuid().toString(), foundHandle+1,
                        descriptor.getUuid().toString(), descriptor.getValue());
                return;
            }


            GattEntry entry = entries.get(runningHandle);
            entry.valueKnown = true;
            entries.set(runningHandle, entry);
            //TODO avoid sending service and characteristic uuid -> handles should be sufficient
            leDescriptorRead(qtObject, descriptor.getCharacteristic().getService().getUuid().toString(),
                    descriptor.getCharacteristic().getUuid().toString(), runningHandle+1,
                    descriptor.getUuid().toString(), descriptor.getValue());
            performServiceDetailDiscoveryForHandle(runningHandle + 1, false);
        }

        public void onDescriptorWrite(android.bluetooth.BluetoothGatt gatt,
                                      android.bluetooth.BluetoothGattDescriptor descriptor,
                                      int status)
        {
            if (status != BluetoothGatt.GATT_SUCCESS)
                Log.w(TAG, "onDescriptorWrite: error " + status);

            int handle = handleForDescriptor(descriptor);

            int errorCode = 0;
            //This must be in sync with QLowEnergyService::ServiceError
            switch (status) {
                case BluetoothGatt.GATT_SUCCESS:
                    errorCode = 0; break; // NoError
                default:
                    errorCode = 3; break; // DescriptorWriteError
            }

            leDescriptorWritten(qtObject, handle+1, descriptor.getValue(), errorCode);
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
        public int endHandle;
    }
    Hashtable<UUID, List<Integer>> uuidToEntry = new Hashtable<UUID, List<Integer>>(100);
    ArrayList<GattEntry> entries = new ArrayList<GattEntry>(100);
    private LinkedList<Integer> servicesToBeDiscovered = new LinkedList<Integer>();

    /*
        Internal helper function
        Returns the handle id for the given characteristic; otherwise returns -1.

        Note that this is the Java handle. The Qt handle is the Java handle +1.
     */
    private int handleForCharacteristic(BluetoothGattCharacteristic characteristic)
    {
        if (characteristic == null)
            return -1;

        List<Integer> handles = uuidToEntry.get(characteristic.getService().getUuid());
        if (handles == null || handles.isEmpty())
            return -1;

        //TODO for now we assume we always want the first service in case of uuid collision
        int serviceHandle = handles.get(0);

        try {
            GattEntry entry = null;
            for (int i = serviceHandle+1; i < entries.size(); i++) {
                entry = entries.get(i);
                switch (entry.type) {
                    case Descriptor:
                    case CharacteristicValue:
                        continue;
                    case Service:
                        break;
                    case Characteristic:
                        if (entry.characteristic == characteristic)
                            return i;
                        break;
                }
            }
        } catch (IndexOutOfBoundsException ex) { /*nothing*/ }
        return -1;
    }

    /*
        Internal helper function
        Returns the handle id for the given descriptor; otherwise returns -1.

        Note that this is the Java handle. The Qt handle is the Java handle +1.
     */
    private int handleForDescriptor(BluetoothGattDescriptor descriptor)
    {
        if (descriptor == null)
            return -1;

        List<Integer> handles = uuidToEntry.get(descriptor.getCharacteristic().getService().getUuid());
        if (handles == null || handles.isEmpty())
            return -1;

        //TODO for now we assume we always want the first service in case of uuid collision
        int serviceHandle = handles.get(0);

        try {
            GattEntry entry = null;
            for (int i = serviceHandle+1; i < entries.size(); i++) {
                entry = entries.get(i);
                switch (entry.type) {
                    case Characteristic:
                    case CharacteristicValue:
                        continue;
                    case Service:
                        break;
                    case Descriptor:
                        if (entry.descriptor == descriptor)
                            return i;
                        break;
                }
            }
        } catch (IndexOutOfBoundsException ex) { }
        return -1;
    }

    private void populateHandles()
    {
        // We introduce the notion of artificial handles. While GATT handles
        // are not exposed on Android they help to quickly identify GATT attributes
        // on the C++ side. The Qt Api will not expose the handles
        GattEntry entry = null;
        List<BluetoothGattService> services = mBluetoothGatt.getServices();
        for (BluetoothGattService service: services) {
            GattEntry serviceEntry = new GattEntry();
            serviceEntry.type = GattEntryType.Service;
            serviceEntry.service = service;
            entries.add(entry);

            // remember handle for the service for later update
            int serviceHandle = entries.size() - 1;

            //some devices may have more than one service with the same uuid
            List<Integer> old = uuidToEntry.get(service.getUuid());
            if (old == null)
                old = new ArrayList<Integer>();
            old.add(entries.size()-1);
            uuidToEntry.put(service.getUuid(), old);

            // add all characteristics
            List<BluetoothGattCharacteristic> charList = service.getCharacteristics();
            for (BluetoothGattCharacteristic characteristic: charList) {
                entry = new GattEntry();
                entry.type = GattEntryType.Characteristic;
                entry.characteristic = characteristic;
                entries.add(entry);

                // this emulates GATT value attributes
                entry = new GattEntry();
                entry.type = GattEntryType.CharacteristicValue;
                entries.add(entry);

                // add all descriptors
                List<BluetoothGattDescriptor> descList = characteristic.getDescriptors();
                for (BluetoothGattDescriptor desc: descList) {
                    entry = new GattEntry();
                    entry.type = GattEntryType.Descriptor;
                    entry.descriptor = desc;
                    entries.add(entry);
                }
            }

            // update endHandle of current service
            serviceEntry.endHandle = entries.size() - 1;
            entries.set(serviceHandle, serviceEntry);
        }

        entries.trimToSize();
    }

    private int currentServiceInDiscovery = -1;
    private int runningHandle = -1;

    private synchronized void resetServiceDetailData()
    {
        runningHandle = -1;
        currentServiceInDiscovery = -1;
        uuidToEntry.clear();
        entries.clear();
        servicesToBeDiscovered.clear();
    }

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
                // we are currently discovering another service
                // we queue the new one up until we finish the previous one
                if (!entry.valueKnown) {
                    servicesToBeDiscovered.add(serviceHandle);
                    Log.w(TAG, "Service discovery already running on another service, " +
                               "queueing request for " + serviceUuid);
                } else {
                    Log.w(TAG, "Service already known");
                }
                return true;
            }

            if (!entry.valueKnown) {
                performServiceDetailDiscoveryForHandle(serviceHandle, true);
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
        int currentEntry = currentServiceInDiscovery;
        GattEntry discoveredService = entries.get(currentServiceInDiscovery);
        discoveredService.valueKnown = true;
        entries.set(currentServiceInDiscovery, discoveredService);

        runningHandle = -1;
        currentServiceInDiscovery = -1;

        leServiceDetailDiscoveryFinished(qtObject, discoveredService.service.getUuid().toString(),
                currentEntry + 1, discoveredService.endHandle + 1);

        if (!servicesToBeDiscovered.isEmpty()) {
            try {
                int nextService = servicesToBeDiscovered.remove();
                performServiceDetailDiscoveryForHandle(nextService, true);
            } catch (IndexOutOfBoundsException ex) {
                Log.w(TAG, "Expected queued service but didn't find any");
                return;
            }
        }
    }

    private synchronized void performServiceDetailDiscoveryForHandle(int nextHandle, boolean searchStarted)
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
                //ex.printStackTrace();
                Log.w(TAG, "Last entry of last service read");
                finishCurrentServiceDiscovery();
                return;
            }

            boolean result = false;
            switch (entry.type) {
                case Characteristic:
                    result = mBluetoothGatt.readCharacteristic(entry.characteristic);
                    try {
                        if (!result) {
                            // add characteristic now since we won't get a read update later one
                            // this is possible when the characteristic is not readable
                            Log.d(TAG, "Non-readable characteristic " + entry.characteristic.getUuid() +
                                    " for service " + entry.characteristic.getService().getUuid());
                            leCharacteristicRead(qtObject, entry.characteristic.getService().getUuid().toString(),
                                    nextHandle + 1, entry.characteristic.getUuid().toString(),
                                    entry.characteristic.getProperties(), entry.characteristic.getValue());
                            performServiceDetailDiscoveryForHandle(runningHandle + 1, false);
                        }
                    } catch (Exception ex)
                    {
                        ex.printStackTrace();
                    }
                    break;
                case CharacteristicValue:
                    // ignore -> nothing to do for this artificial type
                    performServiceDetailDiscoveryForHandle(runningHandle + 1, false);
                    break;
                case Descriptor:
                    result = mBluetoothGatt.readDescriptor(entry.descriptor);
                    if (!result) {
                        // atm all descriptor types are readable
                        Log.d(TAG, "Non-readable descriptor " + entry.descriptor.getUuid() +
                                   " for service/char" + entry.descriptor.getCharacteristic().getService().getUuid() +
                                   "/" + entry.descriptor.getCharacteristic().getUuid());
                        leDescriptorRead(qtObject,
                                entry.descriptor.getCharacteristic().getService().getUuid().toString(),
                                entry.descriptor.getCharacteristic().getUuid().toString(),
                                nextHandle+1, entry.descriptor.getUuid().toString(),
                                entry.descriptor.getValue());
                        performServiceDetailDiscoveryForHandle(runningHandle + 1, false);
                    }
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

    /*************************************************************/
    /* Write Characteristics                                     */
    /*************************************************************/

    public boolean writeCharacteristic(int charHandle, byte[] newValue)
    {
        if (mBluetoothGatt == null)
            return false;

        GattEntry entry = null;
        try {
            entry = entries.get(charHandle-1); //Qt always uses handles+1
        } catch (IndexOutOfBoundsException ex) {
            ex.printStackTrace();
            return false;
        }

        boolean result = entry.characteristic.setValue(newValue);
        if (!result) {
            Log.w(TAG, "BluetoothGattCharacteristic.setValue failed");
            return false;
        }

        return mBluetoothGatt.writeCharacteristic(entry.characteristic);
    }

    /*************************************************************/
    /* Write Descriptors                                         */
    /*************************************************************/

    public boolean writeDescriptor(int descHandle, byte[] newValue)
    {
        if (mBluetoothGatt == null)
            return false;

        GattEntry entry = null;
        try {
            entry = entries.get(descHandle-1); //Qt always uses handles+1
        } catch (IndexOutOfBoundsException ex) {
            ex.printStackTrace();
            return false;
        }

        boolean result = entry.descriptor.setValue(newValue);
        if (!result) {
            Log.w(TAG, "BluetoothGattDescriptor.setValue failed");
            return false;
        }

        return mBluetoothGatt.writeDescriptor(entry.descriptor);
    }


    public native void leConnectionStateChange(long qtObject, int wasErrorTransition, int newState);
    public native void leServicesDiscovered(long qtObject, int errorCode, String uuidList);
    public native void leServiceDetailDiscoveryFinished(long qtObject, final String serviceUuid,
                                                        int startHandle, int endHandle);
    public native void leCharacteristicRead(long qtObject, String serviceUuid,
                                            int charHandle, String charUuid,
                                            int properties, byte[] data);
    public native void leDescriptorRead(long qtObject, String serviceUuid, String charUuid,
                                        int descHandle, String descUuid, byte[] data);
    public native void leCharacteristicWritten(long qtObject, int charHandle, byte[] newData,
                                               int errorCode);
    public native void leDescriptorWritten(long qtObject, int charHandle, byte[] newData,
                                           int errorCode);

}

