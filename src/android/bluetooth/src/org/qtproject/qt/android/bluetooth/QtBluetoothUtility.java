// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

package org.qtproject.qt.android.bluetooth;

import android.Manifest;
import android.os.Build;
import android.content.Context;
import android.content.pm.PackageManager;
import android.content.pm.PackageInfo;
import android.content.res.AssetManager;
import android.content.res.XmlResourceParser;
import android.util.Log;

import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;

class QtBluetoothUtility {

    private static final String TAG = "QtBluetoothUtility";

    // The check if bluetooth scanning requires location is somewhat computationally
    // expensive and doesn't change at runtime, so we cache the result
    private static boolean isScanRequiresLocationChecked = false;
    private static boolean scanRequiresLocation = true;

    // Returns whether or not Location + Location permission is required for Bluetooth scan
    //
    // API-level < 31: returns always true
    //
    // API-level >= 31: returns true if BLUETOOTH_SCAN doesn't have 'neverForLocation' set.
    // Returns false if BLUETOOTH_SCAN has 'neverForLocation' set, or in case of any
    // unexpected failure.
    public static synchronized boolean bluetoothScanRequiresLocation(final Context qtContext)
    {
        Log.d(TAG, "Checking if Location is required for bluetooth scan");
        if (isScanRequiresLocationChecked) {
            Log.d(TAG, "Using cached result for scan needing location: " + scanRequiresLocation);
            return scanRequiresLocation;
        }

        // API-levels below 31 always need location
        if (Build.VERSION.SDK_INT < 31) {
            Log.d(TAG, "SDK version is below 31, assuming Location needed");
            scanRequiresLocation = true;
            isScanRequiresLocationChecked = true;
            return scanRequiresLocation;
        }
        if (qtContext == null) {
            Log.w(TAG, "No context object provided");
            return false;
        }

        // API-levels 31 and above require Location if no 'neverForLocation' assertion
        XmlResourceParser xmlParser = null;
        try {
            // Open the used AndroidManifest.xml for traversing
            final AssetManager assetManager =
                        qtContext.createPackageContext(qtContext.getPackageName(), 0).getAssets();
            xmlParser = assetManager.openXmlResourceParser(0, "AndroidManifest.xml");

            int eventType = xmlParser.getEventType();
            while (eventType != XmlPullParser.END_DOCUMENT) {
                // Check if the current tag is a <uses-permission> tag
                if (eventType == XmlPullParser.START_TAG
                                                && xmlParser.getName().equals("uses-permission")) {
                    String permissionName = null;
                    int usesPermissionFlags = 0;

                    // Loop through the attributes to see if there's BLUETOOTH_SCAN
                    // permission with 'neverForLocation' set
                    for (int i = 0; i < xmlParser.getAttributeCount(); i++) {
                        String attributeName = xmlParser.getAttributeName(i);
                        if (attributeName.equals("name")) {
                            permissionName = xmlParser.getAttributeValue(i);
                        } else if (attributeName.equals("usesPermissionFlags")) {
                            String flagValue = xmlParser.getAttributeValue(i);
                            if (flagValue.startsWith("0x")) {
                                usesPermissionFlags = Integer.parseInt(flagValue.substring(2), 16);
                            } else {
                                usesPermissionFlags = Integer.parseInt(flagValue);
                            }
                        }
                    }
                    if (permissionName.equals(Manifest.permission.BLUETOOTH_SCAN)) {
                        if ((usesPermissionFlags & PackageInfo.REQUESTED_PERMISSION_NEVER_FOR_LOCATION) != 0) {
                            Log.d(TAG, "BLUETOOTH_SCAN with 'neverForLocation' found");
                            scanRequiresLocation = false;
                        } else {
                            Log.d(TAG, "BLUETOOTH_SCAN without 'neverForLocation' found");
                            scanRequiresLocation = true;
                        }
                        isScanRequiresLocationChecked = true;
                        return scanRequiresLocation;
                    }
                }
                eventType = xmlParser.nextToken();
            }
        } catch (Exception ex) {
            Log.w(TAG, "An error occurred while checking Bluetooth's location need: " + ex);
            scanRequiresLocation = false;
        } finally {
            if (xmlParser != null)
                xmlParser.close();
        }
        Log.d(TAG, "BLUETOOTH_SCAN permission not found");
        isScanRequiresLocationChecked = true;
        return scanRequiresLocation;
    }
}
