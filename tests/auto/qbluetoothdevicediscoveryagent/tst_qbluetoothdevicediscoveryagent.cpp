// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include <QtTest/QtTest>

#include <QDebug>
#include <QVariant>
#include <QList>
#include <QLoggingCategory>

#include "../../shared/bttestutil_p.h"
#include <private/qtbluetoothglobal_p.h>
#include <qbluetoothaddress.h>
#include <qbluetoothdevicediscoveryagent.h>
#include <qbluetoothlocaldevice.h>

#if QT_CONFIG(permissions)
#include <QtCore/qcoreapplication.h>
#include <QtCore/qpermissions.h>
#include <QtCore/qnamespace.h>
#endif // permissions

#ifdef Q_OS_ANDROID
#include <QtCore/qjnienvironment.h>
#endif

#include <memory>

using namespace Qt::StringLiterals;

QT_USE_NAMESPACE

/*
 * Some parts of this test require a remote and discoverable Bluetooth
 * device. Setting the BT_TEST_DEVICE environment variable will
 * set the test up to fail if it cannot find a remote device.
 * BT_TEST_DEVICE should contain the address of the device the
 * test expects to find. Ensure that the device is running
 * in discovery mode.
 **/

// Maximum time to for bluetooth device scan
const int MaxScanTime = 5 * 60 * 1000;  // 5 minutes in ms

//Bluez needs at least 10s for a device discovery to be cancelled
const int MaxWaitForCancelTime = 15 * 1000;  // 15 seconds in ms

#ifdef Q_OS_ANDROID
// Android is sometimes unable to cancel immediately
const int WaitBeforeStopTime = 200;
extern void testEnumToString(std::function<void(const char*, int)> callback);
extern void parseScanRecord(QBluetoothDeviceInfo &, jbyteArray);
#endif

class tst_QBluetoothDeviceDiscoveryAgent : public QObject
{
    Q_OBJECT

public:
    tst_QBluetoothDeviceDiscoveryAgent();
    ~tst_QBluetoothDeviceDiscoveryAgent();

public slots:
    void deviceDiscoveryDebug(const QBluetoothDeviceInfo &info);
    void finished();

private slots:
    void initTestCase();

    void tst_invalidBtAddress();

    void tst_startStopDeviceDiscoveries();

    void tst_deviceDiscovery();

    void tst_discoveryTimeout();

    void tst_discoveryMethods();

    void tst_majorMinorMapping();

    void tst_androidScanRecordParsing_data();
    void tst_androidScanRecordParsing();
private:
    qsizetype noOfLocalDevices;
    using DiscoveryAgentPtr = std::unique_ptr<QBluetoothDeviceDiscoveryAgent>;
#if QT_CONFIG(permissions)
    Qt::PermissionStatus permissionStatus = Qt::PermissionStatus::Undetermined;
#endif
};

tst_QBluetoothDeviceDiscoveryAgent::tst_QBluetoothDeviceDiscoveryAgent()
{
    QLoggingCategory::setFilterRules(QStringLiteral("qt.bluetooth* = true"));
    qRegisterMetaType<QBluetoothDeviceDiscoveryAgent::Error>();
#if QT_CONFIG(permissions)
    permissionStatus = qApp->checkPermission(QBluetoothPermission{});

    const bool ciRun = qEnvironmentVariable("QTEST_ENVIRONMENT").split(' ').contains("ci");
    if (!ciRun && permissionStatus == Qt::PermissionStatus::Undetermined) {
        QTestEventLoop loop;
        qApp->requestPermission(QBluetoothPermission{},
                                &loop, [this, &loop](const QPermission &permission) {
            permissionStatus = permission.status();
            loop.exitLoop();
        });
        if (permissionStatus == Qt::PermissionStatus::Undetermined)
            loop.enterLoopMSecs(30000);
    }
#endif // QT_CONFIG(permissions)
}

tst_QBluetoothDeviceDiscoveryAgent::~tst_QBluetoothDeviceDiscoveryAgent()
{
}

void tst_QBluetoothDeviceDiscoveryAgent::initTestCase()
{
    qRegisterMetaType<QBluetoothDeviceInfo>();

    noOfLocalDevices = QBluetoothLocalDevice::allDevices().size();

    if (!noOfLocalDevices)
        return;

    // turn on BT in case it is not on
    QBluetoothLocalDevice *device = new QBluetoothLocalDevice();
    if (device->hostMode() == QBluetoothLocalDevice::HostPoweredOff) {
        QSignalSpy hostModeSpy(device, SIGNAL(hostModeStateChanged(QBluetoothLocalDevice::HostMode)));
        QVERIFY(hostModeSpy.isEmpty());
        device->powerOn();
        int connectTime = 5000;  // ms
        while (hostModeSpy.isEmpty() && connectTime > 0) {
            QTest::qWait(500);
            connectTime -= 500;
        }
        QVERIFY(!hostModeSpy.isEmpty());
    }
    QBluetoothLocalDevice::HostMode hostMode= device->hostMode();
    QVERIFY(hostMode == QBluetoothLocalDevice::HostConnectable
         || hostMode == QBluetoothLocalDevice::HostDiscoverable
         || hostMode == QBluetoothLocalDevice::HostDiscoverableLimitedInquiry);
    delete device;
}

void tst_QBluetoothDeviceDiscoveryAgent::tst_invalidBtAddress()
{
    DiscoveryAgentPtr discoveryAgent(new QBluetoothDeviceDiscoveryAgent(
                QBluetoothAddress(QStringLiteral("11:11:11:11:11:11"))));

    QCOMPARE(discoveryAgent->error(), QBluetoothDeviceDiscoveryAgent::InvalidBluetoothAdapterError);
    discoveryAgent->start();
    QCOMPARE(discoveryAgent->isActive(), false);

#if QT_CONFIG(permissions)
    if (permissionStatus != Qt::PermissionStatus::Granted)
        return;
#endif

    discoveryAgent.reset(new QBluetoothDeviceDiscoveryAgent(QBluetoothAddress()));
    QCOMPARE(discoveryAgent->error(), QBluetoothDeviceDiscoveryAgent::NoError);
    if (!QBluetoothLocalDevice::allDevices().isEmpty()) {
        discoveryAgent->start();
        QCOMPARE(discoveryAgent->isActive(), true);
    }
}

void tst_QBluetoothDeviceDiscoveryAgent::deviceDiscoveryDebug(const QBluetoothDeviceInfo &info)
{
    qDebug() << "Discovered device:" << info.address().toString() << info.name();
}

void tst_QBluetoothDeviceDiscoveryAgent::tst_startStopDeviceDiscoveries()
{
    QBluetoothDeviceDiscoveryAgent discoveryAgent;

    QVERIFY(discoveryAgent.error() == discoveryAgent.NoError);
    QVERIFY(discoveryAgent.errorString().isEmpty());
    QVERIFY(!discoveryAgent.isActive());
    QVERIFY(discoveryAgent.discoveredDevices().isEmpty());

    QSignalSpy finishedSpy(&discoveryAgent, SIGNAL(finished()));
    QSignalSpy cancelSpy(&discoveryAgent, SIGNAL(canceled()));
    QSignalSpy errorSpy(&discoveryAgent,
                        SIGNAL(errorOccurred(QBluetoothDeviceDiscoveryAgent::Error)));

    // Starting case 1: start-stop, expecting cancel signal
    // we should have no errors at this point.
    QVERIFY(errorSpy.isEmpty());

    discoveryAgent.start();
#if QT_CONFIG(permissions)
    if (permissionStatus != Qt::PermissionStatus::Granted) {
        // If bluetooth is OFF, the permission does not get checked (e.g. on Darwin),
        // but not to depend on the order in which errors generated, we
        // do not compare error value with MissionPermissionsError here.
        return;
    }
#endif

    if (errorSpy.isEmpty()) {
        QVERIFY(discoveryAgent.isActive());
        QCOMPARE(discoveryAgent.errorString(), QString());
        QCOMPARE(discoveryAgent.error(), QBluetoothDeviceDiscoveryAgent::NoError);
    } else {
        QCOMPARE(noOfLocalDevices, 0);
        QVERIFY(!discoveryAgent.isActive());
        QVERIFY(!discoveryAgent.errorString().isEmpty());
        QVERIFY(discoveryAgent.error() != QBluetoothDeviceDiscoveryAgent::NoError);
        QSKIP("No local Bluetooth device available. Skipping remaining part of test.");
    }
    // cancel current request.
#ifdef Q_OS_ANDROID
    // Android sometimes can't cancel immediately (~on the same millisecond),
    // but instead a "pending cancel" happens, which means the discovery will be
    // canceled at a later point in time. When this happens the Android backend
    // also emits an immediate errorOccurred(). While this seems to be as intended,
    // as a result many parts of this test function may fail (not always).
    //
    // In this test function we wait some milliseconds between start() and stop() to
    // bypass this behavior difference. This is to avoid complex iffery (Android itself
    // can behave differently every time) and ifdeffery (Q_OS_ANDROID) in the test
    QTest::qWait(WaitBeforeStopTime);
#endif
    discoveryAgent.stop();

    // Wait for up to MaxWaitForCancelTime for the cancel to finish
    QTRY_VERIFY_WITH_TIMEOUT(!cancelSpy.isEmpty(), MaxWaitForCancelTime);

    // we should not be active anymore
    QVERIFY(!discoveryAgent.isActive());
    QVERIFY(errorSpy.isEmpty());
    QCOMPARE(cancelSpy.size(), 1);
    cancelSpy.clear();
    // Starting case 2: start-start-stop, expecting cancel signal
    discoveryAgent.start();
    // we should be active now
    QVERIFY(discoveryAgent.isActive());
    QVERIFY(errorSpy.isEmpty());
    // start again. should this be error?
    discoveryAgent.start();
    QVERIFY(discoveryAgent.isActive());
    QVERIFY(errorSpy.isEmpty());
    // stop
#ifdef Q_OS_ANDROID
    QTest::qWait(WaitBeforeStopTime);
#endif
    discoveryAgent.stop();

    // Wait for up to MaxWaitForCancelTime for the cancel to finish
    QTRY_VERIFY_WITH_TIMEOUT(!cancelSpy.isEmpty(), MaxWaitForCancelTime);

    // we should not be active anymore
    QVERIFY(!discoveryAgent.isActive());
    QVERIFY(errorSpy.isEmpty());

    QCOMPARE(cancelSpy.size(), 1);
    cancelSpy.clear();

    //  Starting case 3: stop
    discoveryAgent.stop();
    QVERIFY(!discoveryAgent.isActive());
    QVERIFY(errorSpy.isEmpty());

    // Don't expect finished signal and no error
    QVERIFY(finishedSpy.isEmpty());
    QVERIFY(discoveryAgent.error() == discoveryAgent.NoError);
    QVERIFY(discoveryAgent.errorString().isEmpty());

    /*
        Starting case 4: start-stop-start-stop:
        We are testing that two subsequent stop() calls reduce total number
        of cancel() signals to 1 if the true cancellation requires
        asynchronous function calls (signal consolidation); otherwise we
        expect 2x cancel() signal.

        Examples are:
            - Bluez4 (event loop needs to run for cancel)
            - Bluez5 (no event loop required)
    */

    bool immediateSignal = false;
    discoveryAgent.start();
    QVERIFY(discoveryAgent.isActive());
    QVERIFY(errorSpy.isEmpty());
    // cancel current request.
#ifdef Q_OS_ANDROID
    QTest::qWait(WaitBeforeStopTime);
#endif
    discoveryAgent.stop();
    //should only have triggered cancel() if stop didn't involve the event loop
    if (cancelSpy.size() == 1) immediateSignal = true;

    // start a new one
    discoveryAgent.start();
    // we should be active now
    QVERIFY(discoveryAgent.isActive());
    QVERIFY(errorSpy.isEmpty());
    // stop
#ifdef Q_OS_ANDROID
    QTest::qWait(WaitBeforeStopTime);
#endif
    discoveryAgent.stop();
    if (immediateSignal)
        QCOMPARE(cancelSpy.size(), 2);

    // Wait for up to MaxWaitForCancelTime for the cancel to finish
    QTRY_VERIFY_WITH_TIMEOUT(!cancelSpy.isEmpty(), MaxWaitForCancelTime);

    // we should not be active anymore
    QVERIFY(!discoveryAgent.isActive());
    QVERIFY(errorSpy.isEmpty());

    // should only have 1 cancel
    if (immediateSignal)
        QCOMPARE(cancelSpy.size(), 2);
    else
        QCOMPARE(cancelSpy.size(), 1);
    cancelSpy.clear();

    // Starting case 5: start-stop-start: expecting finished signal & no cancel
    discoveryAgent.start();
    QVERIFY(discoveryAgent.isActive());
    QVERIFY(errorSpy.isEmpty());
    // cancel current request.
#ifdef Q_OS_ANDROID
    QTest::qWait(WaitBeforeStopTime);
#endif
    discoveryAgent.stop();
    // start a new one
    discoveryAgent.start();
    // we should be active now
    QVERIFY(discoveryAgent.isActive());
    QVERIFY(errorSpy.isEmpty());

    // Wait for up to MaxScanTime for the scan to finish
    QTRY_VERIFY_WITH_TIMEOUT(!finishedSpy.isEmpty(), MaxScanTime);

    // we should not be active anymore
    QVERIFY(!discoveryAgent.isActive());
    QVERIFY(errorSpy.isEmpty());
    // should only have 1 cancel
    QCOMPARE(finishedSpy.size(), 1);

    // On OS X, stop is synchronous (signal will be emitted immediately).
    if (!immediateSignal)
        QVERIFY(cancelSpy.isEmpty());
}

void tst_QBluetoothDeviceDiscoveryAgent::finished()
{
    qDebug() << "Finished called";
}

void tst_QBluetoothDeviceDiscoveryAgent::tst_deviceDiscovery()
{
    {
        //Run test in case of multiple Bluetooth adapters
        QBluetoothLocalDevice localDevice;
        //We will use default adapter if there is no other adapter
        QBluetoothAddress address = localDevice.address();

        QBluetoothDeviceDiscoveryAgent discoveryAgent(address);
        QVERIFY(discoveryAgent.error() == discoveryAgent.NoError);
        QVERIFY(discoveryAgent.errorString().isEmpty());
        QVERIFY(!discoveryAgent.isActive());

        QVERIFY(discoveryAgent.discoveredDevices().isEmpty());

        QSignalSpy finishedSpy(&discoveryAgent, SIGNAL(finished()));
        QSignalSpy errorSpy(&discoveryAgent,
                            SIGNAL(errorOccurred(QBluetoothDeviceDiscoveryAgent::Error)));
        QSignalSpy discoveredSpy(&discoveryAgent, SIGNAL(deviceDiscovered(QBluetoothDeviceInfo)));
//        connect(&discoveryAgent, SIGNAL(finished()), this, SLOT(finished()));
//        connect(&discoveryAgent, SIGNAL(deviceDiscovered(QBluetoothDeviceInfo)),
//                this, SLOT(deviceDiscoveryDebug(QBluetoothDeviceInfo)));

        discoveryAgent.start();

        if (!errorSpy.isEmpty()) {
#if QT_CONFIG(permissions)
            if (permissionStatus == Qt::PermissionStatus::Granted)
#endif

            QCOMPARE(noOfLocalDevices, 0);
            QVERIFY(!discoveryAgent.isActive());
            QSKIP("No local Bluetooth device available. Skipping remaining part of test.");
        }

        QVERIFY(discoveryAgent.isActive());

        // Wait for up to MaxScanTime for the scan to finish
        int scanTime = MaxScanTime;
        while (finishedSpy.isEmpty() && scanTime > 0) {
            QTest::qWait(15000);
            scanTime -= 15000;
        }

        // verify that we are finished
        QVERIFY(!discoveryAgent.isActive());
        // stop
        discoveryAgent.stop();
        QVERIFY(!discoveryAgent.isActive());
        qDebug() << "Scan time left:" << scanTime;
        // Expect finished signal with no error
        QVERIFY(finishedSpy.size() == 1);
        QVERIFY(errorSpy.isEmpty());
        QVERIFY(discoveryAgent.error() == discoveryAgent.NoError);
        QVERIFY(discoveryAgent.errorString().isEmpty());

        // verify that the list is as big as the signals received.
        // discoveredSpy might have more events as some devices are found multiple times,
        // leading to messages like
        // "Almost Duplicate  "88:C6:26:F5:3E:E2" "88-C6-26-F5-3E-E2" - replacing in place"
        QVERIFY(discoveredSpy.size() >= discoveryAgent.discoveredDevices().size());
        // verify that there really was some devices in the array

        const QString remote = qEnvironmentVariable("BT_TEST_DEVICE");
        QBluetoothAddress remoteDevice;
        if (!remote.isEmpty()) {
            remoteDevice = QBluetoothAddress(remote);
            QVERIFY2(!remote.isNull(), "Expecting valid Bluetooth address to be passed via BT_TEST_DEVICE");
        } else {
            qWarning() << "Not using a remote device for testing. Set BT_TEST_DEVICE env to run extended tests involving a remote device";
        }

        if (!remoteDevice.isNull())
            QVERIFY(!discoveredSpy.isEmpty());
        // All returned QBluetoothDeviceInfo should be valid.
        while (!discoveredSpy.isEmpty()) {
            const QBluetoothDeviceInfo info =
                qvariant_cast<QBluetoothDeviceInfo>(discoveredSpy.takeFirst().at(0));
            QVERIFY(info.isValid());
        }
    }
}


void tst_QBluetoothDeviceDiscoveryAgent::tst_discoveryTimeout()
{
    QBluetoothDeviceDiscoveryAgent agent;

    // check default values
#if defined(Q_OS_MACOS) || defined(Q_OS_IOS) || defined(Q_OS_ANDROID) || QT_CONFIG(winrt_bt) \
    || QT_CONFIG(bluez)
    QCOMPARE(agent.lowEnergyDiscoveryTimeout(), 40000);
    agent.setLowEnergyDiscoveryTimeout(-1); // negative ignored
    QCOMPARE(agent.lowEnergyDiscoveryTimeout(), 40000);
    agent.setLowEnergyDiscoveryTimeout(20000);
    QCOMPARE(agent.lowEnergyDiscoveryTimeout(), 20000);
#else
    QCOMPARE(agent.lowEnergyDiscoveryTimeout(), -1);
    agent.setLowEnergyDiscoveryTimeout(20000); // feature not supported -> ignored
    QCOMPARE(agent.lowEnergyDiscoveryTimeout(), -1);
#endif
}

void tst_QBluetoothDeviceDiscoveryAgent::tst_discoveryMethods()
{
    const QBluetoothLocalDevice localDevice;
    if (localDevice.allDevices().size() != 1) {
        // On iOS it returns 0 but we still have working BT.
#ifndef Q_OS_IOS
        QSKIP("This test expects exactly one local device working");
#endif
    }

    const QBluetoothDeviceDiscoveryAgent::DiscoveryMethods
        supportedMethods = QBluetoothDeviceDiscoveryAgent::supportedDiscoveryMethods();

    QVERIFY(supportedMethods != QBluetoothDeviceDiscoveryAgent::NoMethod);

    QBluetoothDeviceDiscoveryAgent::DiscoveryMethod
        unsupportedMethods = QBluetoothDeviceDiscoveryAgent::NoMethod;
    QBluetoothDeviceInfo::CoreConfiguration
        expectedConfiguration = QBluetoothDeviceInfo::BaseRateAndLowEnergyCoreConfiguration;

    if (supportedMethods == QBluetoothDeviceDiscoveryAgent::ClassicMethod) {
        unsupportedMethods = QBluetoothDeviceDiscoveryAgent::LowEnergyMethod;
        expectedConfiguration = QBluetoothDeviceInfo::BaseRateCoreConfiguration;
    } else if (supportedMethods == QBluetoothDeviceDiscoveryAgent::LowEnergyMethod) {
        unsupportedMethods = QBluetoothDeviceDiscoveryAgent::ClassicMethod;
        expectedConfiguration = QBluetoothDeviceInfo::LowEnergyCoreConfiguration;
    }

    QBluetoothDeviceDiscoveryAgent agent;
    QSignalSpy finishedSpy(&agent, SIGNAL(finished()));
    QSignalSpy errorSpy(&agent, SIGNAL(errorOccurred(QBluetoothDeviceDiscoveryAgent::Error)));
    QSignalSpy discoveredSpy(&agent, SIGNAL(deviceDiscovered(QBluetoothDeviceInfo)));

    // NoMethod - should just immediately return:
    agent.start(QBluetoothDeviceDiscoveryAgent::NoMethod);
    QCOMPARE(agent.error(), QBluetoothDeviceDiscoveryAgent::NoError);
    QVERIFY(!agent.isActive());
    QCOMPARE(finishedSpy.size(), 0);
    QCOMPARE(errorSpy.size(), 0);
    QCOMPARE(discoveredSpy.size(), 0);

    if (unsupportedMethods != QBluetoothDeviceDiscoveryAgent::NoMethod) {
        agent.start(unsupportedMethods);
        QCOMPARE(agent.error(), QBluetoothDeviceDiscoveryAgent::UnsupportedDiscoveryMethod);
        QVERIFY(!agent.isActive());
        QVERIFY(finishedSpy.isEmpty());
        QCOMPARE(errorSpy.size(), 1);
        errorSpy.clear();
        QVERIFY(discoveredSpy.isEmpty());
    }

    // Start discovery, probably both Classic and LE methods:
    agent.start(supportedMethods);
#if QT_CONFIG(permissions)
    if (permissionStatus != Qt::PermissionStatus::Granted) {
        QCOMPARE(agent.error(), QBluetoothDeviceDiscoveryAgent::MissingPermissionsError);
        QSKIP("The remaining test requires the Bluetooth permission granted");
    }
#endif
    QVERIFY(agent.isActive());
    QVERIFY(errorSpy.isEmpty());

#define RUN_DISCOVERY(maxTimeout, step, condition) \
    for (int scanTime = maxTimeout; (condition) && scanTime > 0; scanTime -= step) \
        QTest::qWait(step);

    // Wait for up to MaxScanTime for the scan to finish
    const int timeStep = 15000;
    RUN_DISCOVERY(MaxScanTime, timeStep, finishedSpy.isEmpty())

    QVERIFY(!agent.isActive());
    QVERIFY(errorSpy.size() <= 1);

    if (errorSpy.size()) {
        // For example, old iOS device could report it supports LE method,
        // but it actually does not.
        QVERIFY(supportedMethods == QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);
        QCOMPARE(agent.error(), QBluetoothDeviceDiscoveryAgent::UnsupportedDiscoveryMethod);
    } else {
        QVERIFY(finishedSpy.size() == 1);
        QVERIFY(agent.error() == QBluetoothDeviceDiscoveryAgent::NoError);
        QVERIFY(agent.errorString().isEmpty());

        while (!discoveredSpy.isEmpty()) {
            const QBluetoothDeviceInfo info =
                qvariant_cast<QBluetoothDeviceInfo>(discoveredSpy.takeFirst().at(0));
            QVERIFY(info.isValid());
            // on Android we do find devices with unknown configuration
            if (info.coreConfigurations() != QBluetoothDeviceInfo::UnknownCoreConfiguration)
                QVERIFY(info.coreConfigurations() & expectedConfiguration);
        }
    }

    if (unsupportedMethods != QBluetoothDeviceDiscoveryAgent::NoMethod)
        return;

    // Both methods were reported as supported. We already tested them
    // above, now let's test first Classic then LE.
    finishedSpy.clear();
    errorSpy.clear();
    discoveredSpy.clear();

    agent.start(QBluetoothDeviceDiscoveryAgent::ClassicMethod);
    QVERIFY(agent.isActive());
    QVERIFY(errorSpy.isEmpty());
    QCOMPARE(agent.error(), QBluetoothDeviceDiscoveryAgent::NoError);

    RUN_DISCOVERY(MaxScanTime, timeStep, finishedSpy.isEmpty())

    QVERIFY(!agent.isActive());
    QVERIFY(errorSpy.isEmpty());
    QCOMPARE(finishedSpy.size(), 1);

    finishedSpy.clear();
    discoveredSpy.clear();

    agent.start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);
    QVERIFY(agent.isActive());
    QVERIFY(errorSpy.isEmpty());

    RUN_DISCOVERY(MaxScanTime, timeStep, finishedSpy.isEmpty() && errorSpy.isEmpty())

    QVERIFY(!agent.isActive());
    QVERIFY(errorSpy.size() <= 1);

    if (errorSpy.size()) {
        QCOMPARE(agent.error(), QBluetoothDeviceDiscoveryAgent::UnsupportedDiscoveryMethod);
        qDebug() << "QBluetoothDeviceDiscoveryAgent::supportedDiscoveryMethods is inaccurate"
                    " on your platform/with your device, LowEnergyMethod is not supported";
    } else {
        QCOMPARE(agent.error(), QBluetoothDeviceDiscoveryAgent::NoError);
        QCOMPARE(finishedSpy.size(), 1);
    }
}



void tst_QBluetoothDeviceDiscoveryAgent::tst_majorMinorMapping()
{
#if !defined(Q_OS_ANDROID)
    QSKIP("This test is only pertinent to Android!");
#else
    const QMap<QLatin1StringView, int> expectedMapping = {
        // AUDIO_VIDEO
        { "AUDIO_VIDEO_CAMCORDER"_L1, QBluetoothDeviceInfo::Camcorder },
        { "AUDIO_VIDEO_CAR_AUDIO"_L1, QBluetoothDeviceInfo::CarAudio },
        { "AUDIO_VIDEO_HANDSFREE"_L1, QBluetoothDeviceInfo::HandsFreeDevice },
        { "AUDIO_VIDEO_HEADPHONES"_L1, QBluetoothDeviceInfo::Headphones },
        { "AUDIO_VIDEO_HIFI_AUDIO"_L1, QBluetoothDeviceInfo::HiFiAudioDevice },
        { "AUDIO_VIDEO_LOUDSPEAKER"_L1, QBluetoothDeviceInfo::Loudspeaker },
        { "AUDIO_VIDEO_MICROPHONE"_L1, QBluetoothDeviceInfo::Microphone },
        { "AUDIO_VIDEO_PORTABLE_AUDIO"_L1, QBluetoothDeviceInfo::PortableAudioDevice },
        { "AUDIO_VIDEO_SET_TOP_BOX"_L1, QBluetoothDeviceInfo::SetTopBox },
        { "AUDIO_VIDEO_UNCATEGORIZED"_L1, QBluetoothDeviceInfo::UncategorizedAudioVideoDevice },
        { "AUDIO_VIDEO_VCR"_L1, QBluetoothDeviceInfo::Vcr },
        { "AUDIO_VIDEO_VIDEO_CAMERA"_L1, QBluetoothDeviceInfo::VideoCamera },
        { "AUDIO_VIDEO_VIDEO_CONFERENCING"_L1, QBluetoothDeviceInfo::VideoConferencing },
        { "AUDIO_VIDEO_VIDEO_DISPLAY_AND_LOUDSPEAKER"_L1, QBluetoothDeviceInfo::VideoDisplayAndLoudspeaker },
        { "AUDIO_VIDEO_VIDEO_GAMING_TOY"_L1, QBluetoothDeviceInfo::GamingDevice },
        { "AUDIO_VIDEO_VIDEO_MONITOR"_L1, QBluetoothDeviceInfo::VideoMonitor },
        { "AUDIO_VIDEO_WEARABLE_HEADSET"_L1, QBluetoothDeviceInfo::WearableHeadsetDevice },

        // COMPUTER
        { "COMPUTER_UNCATEGORIZED"_L1, QBluetoothDeviceInfo::UncategorizedComputer },
        { "COMPUTER_DESKTOP"_L1, QBluetoothDeviceInfo::DesktopComputer },
        { "COMPUTER_HANDHELD_PC_PDA"_L1, QBluetoothDeviceInfo::HandheldComputer },
        { "COMPUTER_LAPTOP"_L1, QBluetoothDeviceInfo::LaptopComputer },
        { "COMPUTER_PALM_SIZE_PC_PDA"_L1, QBluetoothDeviceInfo::HandheldClamShellComputer },
        { "COMPUTER_SERVER"_L1, QBluetoothDeviceInfo::ServerComputer },
        { "COMPUTER_WEARABLE"_L1, QBluetoothDeviceInfo::WearableComputer },

        // HEALTH
        { "HEALTH_BLOOD_PRESSURE"_L1, QBluetoothDeviceInfo::HealthBloodPressureMonitor },
        { "HEALTH_DATA_DISPLAY"_L1, QBluetoothDeviceInfo::HealthDataDisplay },
        { "HEALTH_GLUCOSE"_L1, QBluetoothDeviceInfo::HealthGlucoseMeter },
        { "HEALTH_PULSE_OXIMETER"_L1, QBluetoothDeviceInfo::HealthPulseOximeter },
        { "HEALTH_PULSE_RATE"_L1, QBluetoothDeviceInfo::HealthStepCounter },
        { "HEALTH_THERMOMETER"_L1, QBluetoothDeviceInfo::HealthThermometer },
        { "HEALTH_UNCATEGORIZED"_L1, QBluetoothDeviceInfo::UncategorizedHealthDevice },
        { "HEALTH_WEIGHING"_L1, QBluetoothDeviceInfo::HealthWeightScale },

        // PHONE
        { "PHONE_CELLULAR"_L1, QBluetoothDeviceInfo::CellularPhone },
        { "PHONE_CORDLESS"_L1, QBluetoothDeviceInfo::CordlessPhone },
        { "PHONE_ISDN"_L1, QBluetoothDeviceInfo::CommonIsdnAccessPhone },
        { "PHONE_MODEM_OR_GATEWAY"_L1, QBluetoothDeviceInfo::WiredModemOrVoiceGatewayPhone },
        { "PHONE_SMART"_L1, QBluetoothDeviceInfo::SmartPhone },
        { "PHONE_UNCATEGORIZED"_L1, QBluetoothDeviceInfo::UncategorizedPhone },

        // TOY
        { "TOY_CONTROLLER"_L1, QBluetoothDeviceInfo::ToyController },
        { "TOY_DOLL_ACTION_FIGURE"_L1, QBluetoothDeviceInfo::ToyDoll },
        { "TOY_GAME"_L1, QBluetoothDeviceInfo::ToyGame },
        { "TOY_ROBOT"_L1, QBluetoothDeviceInfo::ToyRobot },
        { "TOY_UNCATEGORIZED"_L1, QBluetoothDeviceInfo::UncategorizedToy },
        { "TOY_VEHICLE"_L1, QBluetoothDeviceInfo::ToyVehicle },

        // WEARABLE
        { "WEARABLE_GLASSES"_L1, QBluetoothDeviceInfo::WearableGlasses },
        { "WEARABLE_HELMET"_L1, QBluetoothDeviceInfo::WearableHelmet },
        { "WEARABLE_JACKET"_L1, QBluetoothDeviceInfo::WearableJacket },
        { "WEARABLE_PAGER"_L1, QBluetoothDeviceInfo::WearablePager },
        { "WEARABLE_UNCATEGORIZED"_L1, QBluetoothDeviceInfo::UncategorizedWearableDevice },
        { "WEARABLE_WRIST_WATCH"_L1, QBluetoothDeviceInfo::WearableWristWatch },

        // PERIPHERAL
        { "PERIPHERAL_NON_KEYBOARD_NON_POINTING"_L1, QBluetoothDeviceInfo::UncategorizedPeripheral },
        { "PERIPHERAL_KEYBOARD"_L1, QBluetoothDeviceInfo::KeyboardPeripheral },
        { "PERIPHERAL_POINTING"_L1, QBluetoothDeviceInfo::PointingDevicePeripheral },
        { "PERIPHERAL_KEYBOARD_POINTING"_L1, QBluetoothDeviceInfo::KeyboardWithPointingDevicePeripheral }
    };

    qsizetype hits = 0;

    testEnumToString([&](const char *generatedName, int minor) {
        QLatin1StringView generatedNameL1(generatedName);

        auto printOnFailure = qScopeGuard([&] { qDebug("%s", generatedName); });

        QVERIFY(expectedMapping.contains(generatedNameL1));

        int expectedValue = expectedMapping.value(generatedNameL1);
        QCOMPARE(minor, expectedValue);

        printOnFailure.dismiss();

        ++hits;
    });

    QCOMPARE(hits, expectedMapping.size());
#endif
}

void tst_QBluetoothDeviceDiscoveryAgent::tst_androidScanRecordParsing_data()
{
#ifndef Q_OS_ANDROID
    QSKIP("This test only runs on Android");
#endif
    QTest::addColumn<QByteArray>("scanRecord");
    QTest::addColumn<int>("expectedServiceUuidCount");
    QTest::addColumn<bool>("expectName");
    QTest::addColumn<bool>("expectManufacturerData");

    // -----------------------------------------------------------------------
    // Baseline: well-formed scan record containing a 16-bit service UUID,
    // a complete local name, and manufacturer-specific data.
    // All three fields must be parsed correctly.
    //
    // Layout (each field: [nBytes] [ADType] [data...]):
    //   02 01 06        -- Flags (ADType 0x01); not parsed by Qt, skipped
    //   03 03 34 12     -- 16-bit UUID complete (0x1234 in little-endian)
    //   05 09 54 65 73 74 -- Complete local name: "Test"
    //   05 FF E0 00 AB CD -- Manufacturer data: company=0x00E0, payload=0xAB 0xCD
    // -----------------------------------------------------------------------
    QByteArray validRecord = QByteArray::fromHex("020106"      // Flags
                                                 "03033412"    // 16-bit UUID 0x1234
                                                 "050954657374"// "Test"
                                                 "05FFE000ABCD"); // Mfr data
    QTest::newRow("valid_record") << validRecord << 1 << true << true;

    QTest::newRow("zero_length") << QByteArray(1, 0x00) << 0 << false << false;

    QByteArray truncated;
    truncated += char(0x0A);          // nBytes = 10, claims 9 bytes of name data
    truncated += char(0x09);          // ADType: complete local name
    truncated += QByteArray("Hi");    // only 2 bytes available — record is truncated
    QTest::newRow("truncated_record") << truncated << 0 << false << false;
}

#ifdef Q_OS_ANDROID

// Allocates a JVM byte array from a QByteArray. The caller is responsible for
// releasing the returned local reference via env->DeleteLocalRef().
static jbyteArray toJByteArray(QJniEnvironment &env, const QByteArray &data)
{
    jbyteArray arr = env->NewByteArray(jsize(data.size()));
    if (arr && !data.isEmpty())
        env->SetByteArrayRegion(arr, 0, jsize(data.size()),
                                reinterpret_cast<const jbyte *>(data.constData()));
    return arr;
}

#endif

void tst_QBluetoothDeviceDiscoveryAgent::tst_androidScanRecordParsing()
{
#ifdef Q_OS_ANDROID
    QFETCH(QByteArray, scanRecord);
    QFETCH(int, expectedServiceUuidCount);
    QFETCH(bool, expectName);
    QFETCH(bool, expectManufacturerData);

    QBluetoothDeviceInfo info{ QBluetoothAddress("00:11:22:33:44:55"_L1), QString(), 0 };

    QJniEnvironment env;
    jbyteArray jScanRecord = toJByteArray(env, scanRecord);

    parseScanRecord(info, jScanRecord);

    env->DeleteLocalRef(jScanRecord);

    QCOMPARE(info.serviceUuids().size(), expectedServiceUuidCount);

    if (expectedServiceUuidCount > 0) {
        // The valid-record row carries UUID 0x1234 — verify it was parsed correctly.
        QVERIFY(info.serviceUuids().contains(QBluetoothUuid(quint16(0x1234))));
    }

    QCOMPARE(!info.name().isEmpty(), expectName);

    if (expectManufacturerData) {
        // The valid-record row carries manufacturer data for company 0x00E0.
        const auto mfrData = info.manufacturerData();
#ifdef Q_PROCESSOR_X86
        QEXPECT_FAIL("valid_record", "AD type read incorrectly", Abort);
#endif
        QVERIFY(mfrData.contains(0x00E0));
        QCOMPARE(mfrData.value(0x00E0), QByteArray::fromHex("ABCD"));
    }
#endif
}

QTEST_MAIN(tst_QBluetoothDeviceDiscoveryAgent)

#include "tst_qbluetoothdevicediscoveryagent.moc"
