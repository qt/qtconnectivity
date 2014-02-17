/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
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

#include "qbluetoothuuid.h"

#include <QStringList>
#include <QtEndian>

#include <string.h>

QT_BEGIN_NAMESPACE

// Bluetooth base UUID 00000000-0000-1000-8000-00805F9B34FB
// TODO: make more efficient
Q_GLOBAL_STATIC_WITH_ARGS(QUuid, baseUuid, ("{00000000-0000-1000-8000-00805F9B34FB}"))

/*!
    \class QBluetoothUuid
    \inmodule QtBluetooth
    \brief The QBluetoothUuid class generates a UUID for each Bluetooth
    service.

*/

/*!
    \enum QBluetoothUuid::ProtocolUuid

    This enum is a convienience type for Bluetooth protocol UUIDs. Values of this type will be
    implicitly converted into a QBluetoothUuid when necessary.

    \value Sdp      SDP protocol UUID
    \value Udp      UDP protocol UUID
    \value Rfcomm   RFCOMM protocol UUID
    \value Tcp      TCP protocol UUID
    \value TcsBin   Telephony Control Specification UUID
    \value TcsAt    Telephony Control Specification AT UUID
    \value Att      Attribute protocol UUID
    \value Obex     OBEX protocol UUID
    \value Ip       IP protocol UUID
    \value Ftp      FTP protocol UUID
    \value Http     HTTP protocol UUID
    \value Wsp      WSP UUID
    \value Bnep     Bluetooth Network Encapsulation Protocol UUID
    \value Upnp     Extended Service Discovery Profile UUID
    \value Hidp     Human Interface Device Profile UUID
    \value HardcopyControlChannel Hardcopy Cable Replacement Profile UUID
    \value HardcopyDataChannel Hardcopy Cable Replacement Profile UUID
    \value HardcopyNotification Hardcopy Cable Replacement Profile UUID
    \value Avctp    Audio/Video Control Transport Protocol UUID
    \value Avdtp    Audio/Video Distribution Transport Protocol UUID
    \value Cmtp     Common ISDN Access Profile
    \value UdiCPlain UDI protocol UUID
    \value McapControlChannel Multi-Channel Adaptation Protocol UUID
    \value McapDataChannel Multi-Channel Adaptation Protocol UUID
    \value L2cap    L2CAP protocol UUID

    \sa QBluetoothServiceInfo::ProtocolDescriptorList
*/

/*!
    \enum QBluetoothUuid::ServiceClassUuid

    This enum is a convienience type for Bluetooth service class UUIDs. Values of this type will be
    implicitly converted into a QBluetoothUuid when necessary.

    \value PublicBrowseGroup          Public browse group service class. Services which have the public
    browse group in their \l {QBluetoothServiceInfo::BrowseGroupList}{browse group list} are discoverable
    by the remote devices.
    \value ObexObjectPush             OBEX object push service UUID.
    \value ServiceDiscoveryServer
    \value BrowseGroupDescriptor      Browser group descriptor
    \value SerialPort                 Serial Port Profile UUID
    \value LANAccessUsingPPP          LAN Access Profile UUID
    \value DialupNetworking           Dial-up Networking Profile UUID
    \value IrMCSync                   Synchronization Profile UUID
    \value OBEXFileTransfer           File Transfer Profile (FTP) UUID
    \value IrMCSyncCommand            Synchronization Profile UUID
    \value Headset                    Headset Profile (HSP) UUID
    \value AudioSource                Advanced Audio Distribution Profile (A2DP) UUID
    \value AudioSink                  Advanced Audio Distribution Profile (A2DP) UUID
    \value AV_RemoteControlTarget     Audio/Video Remote Control Profile (AVRCP) UUID
    \value AdvancedAudioDistribution  Advanced Audio Distribution Profile (A2DP) UUID
    \value AV_RemoteControl           Audio/Video Remote Control Profile (AVRCP) UUID
    \value AV_RemoteControlController Audio/Video Remote Control Profile UUID
    \value HeadsetAG                  Headset Profile (HSP) UUID
    \value PANU                       Personal Area Networking Profile (PAN) UUID
    \value NAP                        Personal Area Networking Profile (PAN) UUID
    \value GN                         Personal Area Networking Profile (PAN) UUID
    \value DirectPrinting             Basic Printing Profile (BPP) UUID
    \value ReferencePrinting          Related to Basic Printing Profile (BPP) UUID
    \value ImagingResponder           Basic Imaging Profile (BIP) UUID
    \value ImagingAutomaticArchive    Basic Imaging Profile (BIP) UUID
    \value ImagingReferenceObjects    Basic Imaging Profile (BIP) UUID
    \value Handsfree                  Hands-Free Profile (HFP) Service Class Identifier and Profile Identifier
    \value HandsfreeAudioGateway      Hands-free Profile (HFP) UUID
    \value DirectPrintingReferenceObjectsService Basic Printing Profile (BPP) UUID
    \value ReflectedUI                Basic Printing Profile (BPP) UUID
    \value BasicPrinting              Basic Printing Profile (BPP) UUID
    \value PrintingStatus             Basic Printing Profile (BPP) UUID
    \value HumanInterfaceDeviceService Human Interface Device (HID) UUID
    \value HardcopyCableReplacement Hardcopy Cable Replacement Profile (HCRP)
    \value HCRPrint               Hardcopy Cable Replacement Profile (HCRP)
    \value HCRScan                Hardcopy Cable Replacement Profile (HCRP)
    \value SIMAccess              SIM Access Profile (SAP) UUID
    \value PhonebookAccessPCE     Phonebook Access Profile (PBAP) UUID
    \value PhonebookAccessPSE     Phonebook Access Profile (PBAP) UUID
    \value PhonebookAccess        Phonebook Access Profile (PBAP)
    \value HeadsetHS              Headset Profile (HSP) UUID
    \value MessageAccessServer    Message Access Profile (MAP) UUID
    \value MessageNotificationServer Message Access Profile (MAP) UUID
    \value MessageAccessProfile   Message Access Profile (MAP) UUID
    \value PnPInformation         Device Identification (DID) UUID
    \value GenericNetworking      Generic networking
    \value GenericFileTransfer    Generic file transfer
    \value GenericAudio           Generic audio
    \value GenericTelephony       Generic telephone
    \value VideoSource            Video Distribution Profile (VDP)
    \value VideoSink              Video Distribution Profile (VDP)
    \value VideoDistribution      Video Distribution Profile (VDP)
    \value HDP                    Health Device Profile
    \value HDPSource              Health Device Profile
    \value HDPSink                Health Device Profile

    \sa QBluetoothServiceInfo::ServiceClassIds

    \value HardcopyCableReplacement   Hardcopy Cable Replacement Profile (HCRP)
    \value HCRPrint                   Hardcopy Cable Replacement Profile (HCRP)
    \value HCRScan                    Hardcopy Cable Replacement Profile (HCRP)
    \value SIMAccess                  SIM Access Profile (SAP) UUID
    \value PhonebookAccessPCE         Phonebook Access Profile (PBAP) UUID
    \value PhonebookAccessPSE         Phonebook Access Profile (PBAP) UUID
    \value PhonebookAccess            Phonebook Access Profile (PBAP)
    \value HeadsetHS                  Headset Profile (HSP) UUID
    \value MessageAccessServer        Message Access Profile (MAP) UUID
    \value MessageNotificationServer  Message Access Profile (MAP) UUID
    \value MessageAccessProfile       Message Access Profile (MAP) UUID
    \value PnPInformation             Device Identification (DID) UUID
    \value GenericNetworking          Generic networking
    \value GenericFileTransfer        Generic file transfer
    \value GenericAudio               Generic audio
    \value GenericTelephony           Generic telephone
    \value VideoSource                Video Distribution Profile (VDP)
    \value VideoSink                  Video Distribution Profile (VDP)
    \value VideoDistribution          Video Distribution Profile (VDP)
    \value HDP                        Health Device Profile
    \value HDPSource                  Health Device Profile
    \value HDPSink                    Health Device Profile
    \value AlertNotificationService   The Alert Notification service exposes alert information on a device.
    \value BatteryService             The Battery Service exposes the state of a battery within a device.
    \value BloodPressure              This service exposes blood pressure and other data from a blood pressure
                                      monitor intended for healthcare applications.
    \value CurrentTimeService         This service defines how the current time can be exposed using
                                      the Generic Attribute Profile (GATT).
    \value CyclingPower               This service exposes power- and force-related data and
                                      optionally speed- and cadence-related data from a Cycling Power
                                      sensor intended for sports and fitness applications.
    \value CyclingSpeedAndCadence     This service exposes speed-related and cadence-related data from
                                      a Cycling Speed and Cadence sensor intended for fitness applications.
    \value DeviceInformation          The Device Information Service exposes manufacturer and/or vendor
                                      information about a device.
    \value GenericAccess              The generic_access service contains generic information about the device.
                                      All available Characteristics are readonly.
    \value GenericAttribute
    \value Glucose                    This service exposes glucose and other data from a glucose sensor for use
                                      in consumer and professional healthcare applications.
    \value HealthThermometer          The Health Thermometer service exposes temperature and other data from
                                      a thermometer intended for healthcare and fitness applications.
    \value HeartRate                  This service exposes heart rate and other data from a Heart Rate Sensor
                                      intended for fitness applications.
    \value HumanInterfaceDevice       This service exposes the HID reports and other HID data intended for
                                      HID Hosts and HID Devices.
    \value ImmediateAlert             This service exposes a control point to allow a peer device to cause
                                      the device to immediately alert.
    \value LinkLoss                   This service defines behavior when a link is lost between two devices.
    \value LocationAndNavigation      This service exposes location and navigation-related data from
                                      a Location and Navigation sensor intended for outdoor activity applications.
    \value NextDSTChangeService       This service defines how the information about an upcoming DST change can be
                                      exposed using the Generic Attribute Profile (GATT).
    \value PhoneAlertStatusService    This service dexposes the phone alert status when in a connection.
    \value ReferenceTimeUpdateService This service defines how a client can request an update from a reference
                                      time source from a time server using the Generic Attribute Profile (GATT)
    \value RunningSpeedAndCadence     This service exposes speed, cadence and other data from a Running Speed and
                                      Cadence Sensor intended for fitness applications.
    \value ScanParameters             The Scan Parameters Service enables a GATT Server device to expose a characteristic
                                      for the GATT Client to write its scan interval and scan window on the GATT Server device.
    \value TxPower                    This service exposes a device’s current transmit power level when in a connection
*/

/*!
    \enum QBluetoothUuid::CharacteristicId

    This enum is a convienience type for Bluetooth low energy service characteristics class UUIDs. Values of this type
    will be implicitly converted into a QBluetoothUuid when necessary.

    \value AlertCategoryID               Categories of alerts/messages.
    \value AlertCategoryIDBitMask        Categories of alerts/messages.
    \value AlertLevel                    The level of an alert a device is to sound.
                                         If this level is changed while the alert is being sounded,
                                         the new level should take effect.
    \value AlertNotificationControlPoint Control point of the Alert Notification server.
                                         Client can write the command here to request the several
                                         functions toward the server.
    \value AlertStatus                   The Alert Status characteristic defines the Status of alert.
    \value Appearance                    The external appearance of this device. The values are composed
                                         of a category (10-bits) and sub-categories (6-bits).
    \value BatteryLevel                  The current charge level of a battery. 100% represents fully charged
                                         while 0% represents fully discharged.
    \value BloodPressureFeature          The Blood Pressure Feature characteristic is used to describe the supported
                                         features of the Blood Pressure Sensor.
    \value BloodPressureMeasurement      The Blood Pressure Measurement characteristic is a variable length structure
                                         containing a Flags field, a Blood Pressure Measurement Compound Value field,
                                         and contains additional fields such as Time Stamp, Pulse Rate and User ID
                                         as determined by the contents of the Flags field.
    \value BodySensorLocation
    \value BootKeyboardInputReport       The Boot Keyboard Input Report characteristic is used to transfer fixed format
                                         and length Input Report data between a HID Host operating in Boot Protocol Mode
                                         and a HID Service corresponding to a boot keyboard.
    \value BootKeyboardOutputReport      The Boot Keyboard Output Report characteristic is used to transfer fixed format
                                         and length Output Report data between a HID Host operating in Boot Protocol Mode
                                         and a HID Service corresponding to a boot keyboard.
    \value BootMouseInputReport          The Boot Mouse Input Report characteristic is used to transfer fixed format and
                                         length Input Report data between a HID Host operating in Boot Protocol Mode and
                                         a HID Service corresponding to a boot mouse.
    \value CSCFeature                    The CSC (Cycling Speed and Cadence) Feature characteristic is used to describe
                                         the supported features of the Server.
    \value CSCMeasurement                The CSC Measurement characteristic (CSC refers to Cycling Speed and Cadence)
                                         is a variable length structure containing a Flags field and, based on the contents
                                         of the Flags field, may contain one or more additional fields as shown in the tables
                                         below.
    \value CurrentTime
    \value CyclingPowerControlPoint      The Cycling Power Control Point characteristic is used to request a specific function
                                         to be executed on the receiving device.
    \value CyclingPowerFeature           The CP Feature characteristic is used to report a list of features supported by
                                         the device.
    \value CyclingPowerMeasurement       The Cycling Power Measurement characteristic is a variable length structure containing
                                         a Flags field, an Instantaneous Power field and, based on the contents of the Flags
                                         field, may contain one or more additional fields as shown in the table below.
    \value CyclingPowerVector            The Cycling Power Vector characteristic is a variable length structure containing
                                         a Flags fieldand based on the contents of the Flags field, may contain one or more
                                         additional fields as shown in the table below.
    \value DateTime                      The Date Time characteristic is used to represent time.
    \value DayDateTime
    \value DayOfWeek
    \value DeviceName
    \value DSTOffset
    \value ExactTime256
    \value FirmwareRevisionString        The value of this characteristic is a UTF-8 string representing the firmware revision
                                         for the firmware within the device.
    \value GlucoseFeature                The Glucose Feature characteristic is used to describe the supported features
                                         of the Server. When read, the Glucose Feature characteristic returns a value
                                         that is used by a Client to determine the supported features of the Server.
    \value GlucoseMeasurement            The Glucose Measurement characteristic is a variable length structure containing
                                         a Flags field, a Sequence Number field, a Base Time field and, based upon the contents
                                         of the Flags field, may contain a Time Offset field, Glucose Concentration field,
                                         Type-Sample Location field and a Sensor Status Annunciation field.
    \value GlucoseMeasurementContext
    \value HardwareRevisionString        The value of this characteristic is a UTF-8 string representing the hardware revision
                                         for the hardware within the device.
    \value HeartRateControlPoint
    \value HeartRateMeasurement
    \value HIDControlPoint               The HID Control Point characteristic is a control-point attribute that defines the
                                         HID Commands when written.
    \value HIDInformation                The HID Information Characteristic returns the HID attributes when read.
    \value IEEE1107320601RegulatoryCertificationDataList The value of the characteristic is an opaque structure listing
                                         various regulatory and/or certification compliance items to which the device
                                         claims adherence.
    \value IntermediateCuffPressure      This characteristic has the same format as the Blood Pressure Measurement
                                         characteristic.
    \value IntermediateTemperature       The Intermediate Temperature characteristic has the same format as the
                                         Temperature Measurement characteristic
    \value LNControlPoint                The LN Control Point characteristic is used to request a specific function
                                         to be executed on the receiving device.
    \value LNFeature                     The LN Feature characteristic is used to report a list of features supported
                                         by the device.
    \value LocalTimeInformation
    \value LocationAndSpeed              The Location and Speed characteristic is a variable length structure containing
                                         a Flags field and, based on the contents of the Flags field, may contain a combination
                                         of data fields.
    \value ManufacturerNameString        The value of this characteristic is a UTF-8 string representing the name of the
                                         manufacturer of the device.
    \value MeasurementInterval           The Measurement Interval characteristic defines the time between measurements.
    \value ModelNumberString             The value of this characteristic is a UTF-8 string representing the model number
                                         assigned by the device vendor.
    \value Navigation                    The Navigation characteristic is a variable length structure containing a Flags field,
                                         a Bearing field, a Heading field and, based on the contents of the Flags field.
    \value NewAlert                      This characteristic defines the category of the alert and how many new alerts of
                                         that category have occurred in the server device.
    \value PeripheralPreferredConnectionParameters
    \value PeripheralPrivacyFlag
    \value PnPID                         The PnP_ID characteristic returns its value when read using the GATT Characteristic
                                         Value Read procedure.
    \value PositionQuality               The Position Quality characteristic is a variable length structure containing a
                                         Flags field and at least one of the optional data
    \value ProtocolMode                  The Protocol Mode characteristic is used to expose the current protocol mode of
                                         the HID Service with which it is associated, or to set the desired protocol
                                         mode of the HID Service.
    \value ReconnectionAddress           The Information included in this page is informative. The normative descriptions
                                         are contained in the applicable specification.
    \value RecordAccessControlPoint      This control point is used with a service to provide basic management functionality
                                         for the Glucose Sensor patient record database.
    \value ReferenceTimeInformation
    \value Report                        The Report characteristic is used to exchange data between a HID Device and a HID Host.
    \value ReportMap                     Only a single instance of this characteristic exists as part of a HID Service.
    \value RingerControlPoint            The Ringer Control Point characteristic defines the Control Point of Ringer.
    \value RingerSetting                 The Ringer Setting characteristic defines the Setting of the Ringer.
    \value RSCFeature                    The RSC (Running Speed and Cadence) Feature characteristic is used to describe the
                                         supported features of the Server.
    \value RSCMeasurement                RSC refers to Running Speed and Cadence.
    \value SCControlPoint                The SC Control Point characteristic is used to request a specific function to be
                                         executed on the receiving device.
    \value ScanIntervalWindow            The Scan Interval Window characteristic is used to store the scan parameters of
                                         the GATT Client.
    \value ScanRefresh                   The Scan Refresh characteristic is used to notify the Client that the Server
                                         requires the Scan Interval Window characteristic to be written with the latest
                                         values upon notification.
    \value SensorLocation                The Sensor Location characteristic is used to expose the location of the sensor.
    \value SerialNumberString            The value of this characteristic is a variable-length UTF-8 string representing
                                         the serial number for a particular instance of the device.
    \value ServiceChanged
    \value SoftwareRevisionString        The value of this characteristic is a UTF-8 string representing the software
                                         revision for the software within the device.
    \value SupportedNewAlertCategory     Category that the server supports for new alert.
    \value SupportedUnreadAlertCategory  Category that the server supports for unread alert.
    \value SystemID                      If the system ID is based of a Bluetooth Device Address with a Company Identifier
                                         (OUI) is 0x123456 and the Company Assigned Identifier is 0x9ABCDE, then the System
                                         Identifier is required to be 0x123456FFFE9ABCDE.
    \value TemperatureMeasurement        The Temperature Measurement characteristic is a variable length structure containing
                                         a Flags field, a Temperature Measurement Value field and, based upon the contents of
                                         the Flags field, optionally a Time Stamp field and/or a Temperature Type field.
    \value TemperatureType               The Temperature Type characteristic is an enumeration that indicates where the
                                         temperature was measured.
    \value TimeAccuracy
    \value TimeSource
    \value TimeUpdateControlPoint
    \value TimeUpdateState
    \value TimeWithDST
    \value TimeZone
    \value TxPowerLevel                  The value of the characteristic is a signed 8 bit integer that has a fixed point
                                         exponent of 0.
    \value UnreadAlertStatus             This characteristic shows how many numbers of unread alerts exist in the specific
                                         category in the device.
*/

/*!
    \enum QBluetoothUuid::DescriptorID

    This enum is a convienience type for Bluetooth low energy service descriptor class UUIDs. Values of this type
    will be implicitly converted into a QBluetoothUuid when necessary.

    \value CharacteristicExtendedProperties  Descriptor defines additional Characteristic Properties.
    \value CharacteristicUserDescription     Descriptor provides a textual user description for a characteristic value.
    \value ClientCharacteristicConfiguration Descriptor defines how the characteristic may be configured by a specific client.
    \value ServerCharacteristicConfiguration Descriptor defines how the characteristic descriptor is associated with may be
                                             configured for the server.
    \value CharacteristicPresentationFormat  Descriptor defines the format of the Characteristic Value.
    \value CharacteristicAggregateFormat     Descriptor defines the format of an aggregated Characteristic Value.
    \value ValidRange                        descriptor is used for defining the range of a characteristics.
                                             Two mandatory fields are contained (upper and lower bounds) which define the range.
    \value ExternalReportReference           Allows a HID Host to map information from the Report Map characteristic value for
                                             Input Report, Output Report or Feature Report data to the Characteristic UUID of
                                             external service characteristics used to transfer the associated data.
    \value ReportReference                   Mapping information in the form of a Report ID and Report Type which maps the
                                             current parent characteristic to the Report ID(s) and Report Type (s) defined
                                             within the Report Map characteristic.
*/

/*!
    Constructs a new null Bluetooth UUID.
*/
QBluetoothUuid::QBluetoothUuid()
{
}

/*!
    Constructs a new Bluetooth UUID from the protocol \a uuid.
*/
QBluetoothUuid::QBluetoothUuid(ProtocolUuid uuid)
:   QUuid(uuid, baseUuid()->data2,
          baseUuid()->data3, baseUuid()->data4[0], baseUuid()->data4[1],
          baseUuid()->data4[2], baseUuid()->data4[3], baseUuid()->data4[4], baseUuid()->data4[5],
          baseUuid()->data4[6], baseUuid()->data4[7])
{
}

/*!
    Constructs a new Bluetooth UUID from the service class \a uuid.
*/
QBluetoothUuid::QBluetoothUuid(ServiceClassUuid uuid)
:   QUuid(uuid, baseUuid()->data2, baseUuid()->data3, baseUuid()->data4[0], baseUuid()->data4[1],
          baseUuid()->data4[2], baseUuid()->data4[3], baseUuid()->data4[4], baseUuid()->data4[5],
          baseUuid()->data4[6], baseUuid()->data4[7])
{
}

/*!
    Constructs a new Bluetooth UUID from the characteristic id \a uuid.
*/
QBluetoothUuid::QBluetoothUuid(CharacteristicId uuid)
:   QUuid(uuid, baseUuid()->data2, baseUuid()->data3, baseUuid()->data4[0], baseUuid()->data4[1],
          baseUuid()->data4[2], baseUuid()->data4[3], baseUuid()->data4[4], baseUuid()->data4[5],
          baseUuid()->data4[6], baseUuid()->data4[7])
{
}

/*!
    Constructs a new Bluetooth UUID from the 16 bit \a uuid.
*/
QBluetoothUuid::QBluetoothUuid(quint16 uuid)
:   QUuid(uuid, baseUuid()->data2, baseUuid()->data3, baseUuid()->data4[0], baseUuid()->data4[1],
          baseUuid()->data4[2], baseUuid()->data4[3], baseUuid()->data4[4], baseUuid()->data4[5],
          baseUuid()->data4[6], baseUuid()->data4[7])
{
}

/*!
    Constructs a new Bluetooth UUID from the 32 bit \a uuid.
*/
QBluetoothUuid::QBluetoothUuid(quint32 uuid)
:   QUuid(uuid, baseUuid()->data2, baseUuid()->data3, baseUuid()->data4[0], baseUuid()->data4[1],
          baseUuid()->data4[2], baseUuid()->data4[3], baseUuid()->data4[4], baseUuid()->data4[5],
          baseUuid()->data4[6], baseUuid()->data4[7])
{
}

/*!
    Constructs a new Bluetooth UUID from the 128 bit \a uuid.
*/
QBluetoothUuid::QBluetoothUuid(quint128 uuid)
{
    // TODO: look at the memcpy(), should not be needed
    quint32 tmp32;
    memcpy(&tmp32, &uuid.data[0], 4);
    data1 = qFromBigEndian<quint32>(tmp32);

    quint16 tmp16;
    memcpy(&tmp16, &uuid.data[4], 2);
    data2 = qFromBigEndian<quint16>(tmp16);

    memcpy(&tmp16, &uuid.data[6], 2);
    data3 = qFromBigEndian<quint16>(tmp16);

    memcpy(data4, &uuid.data[8], 8);
}

/*!
    Constructs a new Bluetooth UUID from the \a uuid string.

    The string must be in the form XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX.
*/
QBluetoothUuid::QBluetoothUuid(const QString &uuid)
:   QUuid(uuid)
{
}

/*!
    Constructs a new Bluetooth UUID that is a copy of \a uuid.
*/
QBluetoothUuid::QBluetoothUuid(const QBluetoothUuid &uuid)
:   QUuid(uuid)
{
}

/*!
    Constructs a new Bluetooth UUID that is a copy of \a uuid.
*/
QBluetoothUuid::QBluetoothUuid(const QUuid &uuid)
:   QUuid(uuid)
{
}

/*!
    Destroys the Bluetooth UUID.
*/
QBluetoothUuid::~QBluetoothUuid()
{
}

/*!
    Returns the minimum size in bytes that this UUID can be represented in.  For non-null UUIDs 2,
    4 or 16 is returned.  0 is returned for null UUIDs.

    \sa isNull(), toUInt16(), toUInt32(), toUInt128()
*/
int QBluetoothUuid::minimumSize() const
{
    if (data2 == baseUuid()->data2 && data3 == baseUuid()->data3 &&
        memcmp(data4, baseUuid()->data4, 8) == 0) {
        // 16 or 32 bit Bluetooth UUID
        if (data1 & 0xFFFF0000)
            return 4;
        else
            return 2;
    }

    if (isNull())
        return 0;

    return 16;
}

/*!
    Returns the 16 bit representation of this UUID. If \a ok is passed, it is set to true if the
    conversion is possible, otherwise it is set to false. The return value is undefined if \a ok is
    set to false.
*/
quint16 QBluetoothUuid::toUInt16(bool *ok) const
{
    if (data1 & 0xFFFF0000 || data2 != baseUuid()->data2 || data3 != baseUuid()->data3 ||
        memcmp(data4, baseUuid()->data4, 8) != 0) {
        // not convertable to 16 bit Bluetooth UUID.
        if (ok)
            *ok = false;
        return 0;
    }

    if (ok)
        *ok = true;

    return data1;
}

/*!
    Returns the 32 bit representation of this UUID. If \a ok is passed, it is set to true if the
    conversion is possible, otherwise it is set to false. The return value is undefined if \a ok is
    set to false.
*/
quint32 QBluetoothUuid::toUInt32(bool *ok) const
{
    if (data2 != baseUuid()->data2 || data3 != baseUuid()->data3 ||
        memcmp(data4, baseUuid()->data4, 8) != 0) {
        // not convertable to 32 bit Bluetooth UUID.
        if (ok)
            *ok = false;
        return 0;
    }

    if (ok)
        *ok = true;

    return data1;
}

/*!
    Returns the 128 bit representation of this UUID.
*/
quint128 QBluetoothUuid::toUInt128() const
{
    quint128 uuid;

    quint32 tmp32 = qToBigEndian<quint32>(data1);
    memcpy(&uuid.data[0], &tmp32, 4);

    quint16 tmp16 = qToBigEndian<quint16>(data2);
    memcpy(&uuid.data[4], &tmp16, 2);

    tmp16 = qToBigEndian<quint16>(data3);
    memcpy(&uuid.data[6], &tmp16, 2);

    memcpy(&uuid.data[8], data4, 8);

    return uuid;
}

/*!
    Returns true if \a other is equal to this Bluetooth UUID, otherwise false.
*/
bool QBluetoothUuid::operator==(const QBluetoothUuid &other) const
{
    return QUuid::operator==(other);
}

QT_END_NAMESPACE
