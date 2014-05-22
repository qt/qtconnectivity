/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
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
#include "qbluetoothservicediscoveryagent.h"

#include <QStringList>
#include <QtEndian>

#include <string.h>

QT_BEGIN_NAMESPACE

// Bluetooth base UUID 00000000-0000-1000-8000-00805F9B34FB
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

    This enum is a convienience type for Bluetooth service class and profile UUIDs. Values of this type will be
    implicitly converted into a QBluetoothUuid when necessary. Some UUIDs refer to service class ids, others to profile
    ids and some can be used as both. In general, profile UUIDs shall only be used in a
    \l QBluetoothServiceInfo::BluetoothProfileDescriptorList attribute and service class UUIDs shall only be used
    in a \l QBluetoothServiceInfo::ServiceClassIds attribute. If the UUID is marked as profile and service class UUID
    it can be used as a value for either of the above service attributes. Such a dual use has historical reasons
    but is no longer permissible for newer UUIDs.

    The list below explicitly states as what type each UUID shall be used.

    \value ServiceDiscoveryServer Service discovery server UUID (service)
    \value BrowseGroupDescriptor Browser group descriptor (service)
    \value PublicBrowseGroup     Public browse group service class. Services which have the public
                                 browse group in their \l {QBluetoothServiceInfo::BrowseGroupList}{browse group list}
                                 are discoverable by the remote devices.
    \value SerialPort            Serial Port Profile UUID (service & profile)
    \value LANAccessUsingPPP     LAN Access Profile UUID (service & profile)
    \value DialupNetworking      Dial-up Networking Profile UUID (service & profile)
    \value IrMCSync              Synchronization Profile UUID (service & profile)
    \value ObexObjectPush        OBEX object push service UUID (service & profile)
    \value OBEXFileTransfer      File Transfer Profile (FTP) UUID (service & profile)
    \value IrMCSyncCommand       Synchronization Profile UUID (profile)
    \value Headset               Headset Profile (HSP) UUID (service & profile)
    \value AudioSource           Advanced Audio Distribution Profile (A2DP) UUID (service)
    \value AudioSink             Advanced Audio Distribution Profile (A2DP) UUID (service)
    \value AV_RemoteControlTarget Audio/Video Remote Control Profile (AVRCP) UUID (service)
    \value AdvancedAudioDistribution Advanced Audio Distribution Profile (A2DP) UUID (profile)
    \value AV_RemoteControl       Audio/Video Remote Control Profile (AVRCP) UUID (service & profile)
    \value AV_RemoteControlController Audio/Video Remote Control Profile UUID (service)
    \value HeadsetAG              Headset Profile (HSP) UUID (service)
    \value PANU                   Personal Area Networking Profile (PAN) UUID (service & profile)
    \value NAP                    Personal Area Networking Profile (PAN) UUID (service & profile)
    \value GN                     Personal Area Networking Profile (PAN) UUID (service & profile)
    \value DirectPrinting         Basic Printing Profile (BPP) UUID (service)
    \value ReferencePrinting      Related to Basic Printing Profile (BPP) UUID (service)
    \value BasicImage             Basic Imaging Profile (BIP) UUID (profile)
    \value ImagingResponder       Basic Imaging Profile (BIP) UUID (service)
    \value ImagingAutomaticArchive Basic Imaging Profile (BIP) UUID (service)
    \value ImagingReferenceObjects Basic Imaging Profile (BIP) UUID (service)
    \value Handsfree              Hands-Free Profile (HFP) UUID (service & profile)
    \value HandsfreeAudioGateway  Hands-Free Audio Gateway (HFP) UUID (service)
    \value DirectPrintingReferenceObjectsService Basic Printing Profile (BPP) UUID (service)
    \value ReflectedUI            Basic Printing Profile (BPP) UUID (service)
    \value BasicPrinting          Basic Printing Profile (BPP) UUID (profile)
    \value PrintingStatus         Basic Printing Profile (BPP) UUID (service)
    \value HumanInterfaceDeviceService Human Interface Device (HID) UUID (service & profile)
    \value HardcopyCableReplacement Hardcopy Cable Replacement Profile (HCRP) (profile)
    \value HCRPrint               Hardcopy Cable Replacement Profile (HCRP) (service)
    \value HCRScan                Hardcopy Cable Replacement Profile (HCRP) (service)
    \value SIMAccess              SIM Access Profile (SAP) UUID (service and profile)
    \value PhonebookAccessPCE     Phonebook Access Profile (PBAP) UUID (service)
    \value PhonebookAccessPSE     Phonebook Access Profile (PBAP) UUID (service)
    \value PhonebookAccess        Phonebook Access Profile (PBAP) (profile)
    \value HeadsetHS              Headset Profile (HSP) UUID (service)
    \value MessageAccessServer    Message Access Profile (MAP) UUID (service)
    \value MessageNotificationServer Message Access Profile (MAP) UUID (service)
    \value MessageAccessProfile   Message Access Profile (MAP) UUID (profile)
    \value GNSS                   Global Navigation Satellite System UUID (profile)
    \value GNSSServer             Global Navigation Satellite System Server (UUID) (service)
    \value Display3D              3D Synchronization Display UUID (service)
    \value Glasses3D              3D Synchronization Glasses UUID (service)
    \value Synchronization3D      3D Synchronization UUID (profile)
    \value MPSProfile             Multi-Profile Specification UUID (profile)
    \value MPSService             Multi-Profile Specification UUID (service)
    \value PnPInformation         Device Identification (DID) UUID (service & profile)
    \value GenericNetworking      Generic networking UUID (service)
    \value GenericFileTransfer    Generic file transfer UUID (service)
    \value GenericAudio           Generic audio UUID (service)
    \value GenericTelephony       Generic telephone UUID (service)
    \value VideoSource            Video Distribution Profile (VDP) UUID (service)
    \value VideoSink              Video Distribution Profile (VDP) UUID (service)
    \value VideoDistribution      Video Distribution Profile (VDP) UUID (profile)
    \value HDP                    Health Device Profile (HDP) UUID (profile)
    \value HDPSource              Health Device Profile Source (HDP) UUID (service)
    \value HDPSink                Health Device Profile Sink (HDP) UUID (service)

    \sa QBluetoothServiceInfo::ServiceClassIds
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
    Returns a human-readable and translated name for the given service class
    represented by \a uuid.

    \sa QBluetoothUuid::ServiceClassUuid
    \since Qt 5.4
 */
QString QBluetoothUuid::serviceClassToString(QBluetoothUuid::ServiceClassUuid uuid)
{
    switch (uuid) {
    case QBluetoothUuid::ServiceDiscoveryServer: return QBluetoothServiceDiscoveryAgent::tr("Service Discovery");
    case QBluetoothUuid::BrowseGroupDescriptor: return QBluetoothServiceDiscoveryAgent::tr("Browse Group Descriptor");
    case QBluetoothUuid::PublicBrowseGroup: return QBluetoothServiceDiscoveryAgent::tr("Public Browse Group");
    case QBluetoothUuid::SerialPort: return QBluetoothServiceDiscoveryAgent::tr("Serial Port Profile");
    case QBluetoothUuid::LANAccessUsingPPP: return QBluetoothServiceDiscoveryAgent::tr("LAN Access Profile");
    case QBluetoothUuid::DialupNetworking: return QBluetoothServiceDiscoveryAgent::tr("Dial-Up Networking");
    case QBluetoothUuid::IrMCSync: return QBluetoothServiceDiscoveryAgent::tr("Synchronization");
    case QBluetoothUuid::ObexObjectPush: return QBluetoothServiceDiscoveryAgent::tr("Object Push");
    case QBluetoothUuid::OBEXFileTransfer: return QBluetoothServiceDiscoveryAgent::tr("File Transfer");
    case QBluetoothUuid::IrMCSyncCommand: return QBluetoothServiceDiscoveryAgent::tr("Synchronization Command");
    case QBluetoothUuid::Headset: return QBluetoothServiceDiscoveryAgent::tr("Headset");
    case QBluetoothUuid::AudioSource: return QBluetoothServiceDiscoveryAgent::tr("Audio Source");
    case QBluetoothUuid::AudioSink: return QBluetoothServiceDiscoveryAgent::tr("Audio Sink");
    case QBluetoothUuid::AV_RemoteControlTarget: return QBluetoothServiceDiscoveryAgent::tr("Audio/Video Remote Control Target");
    case QBluetoothUuid::AdvancedAudioDistribution: return QBluetoothServiceDiscoveryAgent::tr("Advanced Audio Distribution");
    case QBluetoothUuid::AV_RemoteControl: return QBluetoothServiceDiscoveryAgent::tr("Audio/Video Remote Control");
    case QBluetoothUuid::AV_RemoteControlController: return QBluetoothServiceDiscoveryAgent::tr("Audio/Video Remote Control Controller");
    case QBluetoothUuid::HeadsetAG: return QBluetoothServiceDiscoveryAgent::tr("Headset AG");
    case QBluetoothUuid::PANU: return QBluetoothServiceDiscoveryAgent::tr("Personal Area Networking (PANU)");
    case QBluetoothUuid::NAP: return QBluetoothServiceDiscoveryAgent::tr("Personal Area Networking (NAP)");
    case QBluetoothUuid::GN: return QBluetoothServiceDiscoveryAgent::tr("Personal Area Networking (GN)");
    case QBluetoothUuid::DirectPrinting: return QBluetoothServiceDiscoveryAgent::tr("Basic Direct Printing (BPP)");
    case QBluetoothUuid::ReferencePrinting: return QBluetoothServiceDiscoveryAgent::tr("Basic Reference Printing (BPP)");
    case QBluetoothUuid::BasicImage: return QBluetoothServiceDiscoveryAgent::tr("Basic Imaging Profile");
    case QBluetoothUuid::ImagingResponder: return QBluetoothServiceDiscoveryAgent::tr("Basic Imaging Responder");
    case QBluetoothUuid::ImagingAutomaticArchive: return QBluetoothServiceDiscoveryAgent::tr("Basic Imaging Archive");
    case QBluetoothUuid::ImagingReferenceObjects: return QBluetoothServiceDiscoveryAgent::tr("Basic Imaging Ref Objects");
    case QBluetoothUuid::Handsfree: return QBluetoothServiceDiscoveryAgent::tr("Hands-Free");
    case QBluetoothUuid::HandsfreeAudioGateway: return QBluetoothServiceDiscoveryAgent::tr("Hands-Free AG");
    case QBluetoothUuid::DirectPrintingReferenceObjectsService: return QBluetoothServiceDiscoveryAgent::tr("Basic Printing RefObject Service");
    case QBluetoothUuid::ReflectedUI: return QBluetoothServiceDiscoveryAgent::tr("Basic Printing Reflected UI");
    case QBluetoothUuid::BasicPrinting: return QBluetoothServiceDiscoveryAgent::tr("Basic Printing");
    case QBluetoothUuid::PrintingStatus: return QBluetoothServiceDiscoveryAgent::tr("Basic Printing Status");
    case QBluetoothUuid::HumanInterfaceDeviceService: return QBluetoothServiceDiscoveryAgent::tr("Human Interface Device");
    case QBluetoothUuid::HardcopyCableReplacement: return QBluetoothServiceDiscoveryAgent::tr("Hardcopy Cable Replacement");
    case QBluetoothUuid::HCRPrint: return QBluetoothServiceDiscoveryAgent::tr("Hardcopy Cable Replacement Print");
    case QBluetoothUuid::HCRScan: return QBluetoothServiceDiscoveryAgent::tr("Hardcopy Cable Replacement Scan");
    case QBluetoothUuid::SIMAccess: return QBluetoothServiceDiscoveryAgent::tr("SIM Access Server");
    case QBluetoothUuid::PhonebookAccessPCE: return QBluetoothServiceDiscoveryAgent::tr("Phonebook Access PCE");
    case QBluetoothUuid::PhonebookAccessPSE: return QBluetoothServiceDiscoveryAgent::tr("Phonebook Access PSE");
    case QBluetoothUuid::PhonebookAccess: return QBluetoothServiceDiscoveryAgent::tr("Phonebook Access");
    case QBluetoothUuid::HeadsetHS: return QBluetoothServiceDiscoveryAgent::tr("Headset HS");
    case QBluetoothUuid::MessageAccessServer: return QBluetoothServiceDiscoveryAgent::tr("Message Access Server");
    case QBluetoothUuid::MessageNotificationServer: return QBluetoothServiceDiscoveryAgent::tr("Message Notification Server");
    case QBluetoothUuid::MessageAccessProfile: return QBluetoothServiceDiscoveryAgent::tr("Message Access");
    case QBluetoothUuid::GNSS: return QBluetoothServiceDiscoveryAgent::tr("Global Navigation Satellite System");
    case QBluetoothUuid::GNSSServer: return QBluetoothServiceDiscoveryAgent::tr("Global Navigation Satellite System Server");
    case QBluetoothUuid::Display3D: return QBluetoothServiceDiscoveryAgent::tr("3D Synchronization Display");
    case QBluetoothUuid::Glasses3D: return QBluetoothServiceDiscoveryAgent::tr("3D Synchronization Glasses");
    case QBluetoothUuid::Synchronization3D: return QBluetoothServiceDiscoveryAgent::tr("3D Synchronization");
    case QBluetoothUuid::MPSProfile: return QBluetoothServiceDiscoveryAgent::tr("Multi-Profile Specification (Profile)");
    case QBluetoothUuid::MPSService: return QBluetoothServiceDiscoveryAgent::tr("Multi-Profile Specification");
    case QBluetoothUuid::PnPInformation: return QBluetoothServiceDiscoveryAgent::tr("Device Identification");
    case QBluetoothUuid::GenericNetworking: return QBluetoothServiceDiscoveryAgent::tr("Generic Networking");
    case QBluetoothUuid::GenericFileTransfer: return QBluetoothServiceDiscoveryAgent::tr("Generic File Transfer");
    case QBluetoothUuid::GenericAudio: return QBluetoothServiceDiscoveryAgent::tr("Generic Audio");
    case QBluetoothUuid::GenericTelephony: return QBluetoothServiceDiscoveryAgent::tr("Generic Telephony");
    case QBluetoothUuid::VideoSource: return QBluetoothServiceDiscoveryAgent::tr("Video Source");
    case QBluetoothUuid::VideoSink: return QBluetoothServiceDiscoveryAgent::tr("Video Sink");
    case QBluetoothUuid::VideoDistribution: return QBluetoothServiceDiscoveryAgent::tr("Video Distribution");
    case QBluetoothUuid::HDP: return QBluetoothServiceDiscoveryAgent::tr("Health Device");
    case QBluetoothUuid::HDPSource: return QBluetoothServiceDiscoveryAgent::tr("Health Device Source");
    case QBluetoothUuid::HDPSink: return QBluetoothServiceDiscoveryAgent::tr("Health Device Sink");
    default:
        break;
    }

    return QString();
}


/*!
    Returns a human-readable and translated name for the given protocol
    represented by \a uuid.

    \sa QBluetoothUuid::ProtocolUuid

    \since Qt 5.4
 */
QString QBluetoothUuid::protocolToString(QBluetoothUuid::ProtocolUuid uuid)
{
    switch (uuid) {
    case QBluetoothUuid::Sdp: return QBluetoothServiceDiscoveryAgent::tr("Service Discovery Protocol");
    case QBluetoothUuid::Udp: return QBluetoothServiceDiscoveryAgent::tr("User Datagram Protocol");
    case QBluetoothUuid::Rfcomm: return QBluetoothServiceDiscoveryAgent::tr("Radio Frequency Communication");
    case QBluetoothUuid::Tcp: return QBluetoothServiceDiscoveryAgent::tr("Transmission Control Protocol");
    case QBluetoothUuid::TcsBin: return QBluetoothServiceDiscoveryAgent::tr("Telephony Control Specification - Binary");
    case QBluetoothUuid::TcsAt: return QBluetoothServiceDiscoveryAgent::tr("Telephony Control Specification - AT");
    case QBluetoothUuid::Att: return QBluetoothServiceDiscoveryAgent::tr("Attribute Protocol");
    case QBluetoothUuid::Obex: return QBluetoothServiceDiscoveryAgent::tr("Object Exchange Protocol");
    case QBluetoothUuid::Ip: return QBluetoothServiceDiscoveryAgent::tr("Internet Protocol");
    case QBluetoothUuid::Ftp: return QBluetoothServiceDiscoveryAgent::tr("File Transfer Protocol");
    case QBluetoothUuid::Http: return QBluetoothServiceDiscoveryAgent::tr("Hypertext Transfer Protocol");
    case QBluetoothUuid::Wsp: return QBluetoothServiceDiscoveryAgent::tr("Wireless Short Packet Protocol");
    case QBluetoothUuid::Bnep: return QBluetoothServiceDiscoveryAgent::tr("Bluetooth Network Encapsulation Protocol");
    case QBluetoothUuid::Upnp: return QBluetoothServiceDiscoveryAgent::tr("Extended Service Discovery Protocol");
    case QBluetoothUuid::Hidp: return QBluetoothServiceDiscoveryAgent::tr("Human Interface Device Protocol");
    case QBluetoothUuid::HardcopyControlChannel: return QBluetoothServiceDiscoveryAgent::tr("Hardcopy Control Channel");
    case QBluetoothUuid::HardcopyDataChannel: return QBluetoothServiceDiscoveryAgent::tr("Hardcopy Data Channel");
    case QBluetoothUuid::HardcopyNotification: return QBluetoothServiceDiscoveryAgent::tr("Hardcopy Notification");
    case QBluetoothUuid::Avctp: return QBluetoothServiceDiscoveryAgent::tr("Audio/Video Control Transport Protocol");
    case QBluetoothUuid::Avdtp: return QBluetoothServiceDiscoveryAgent::tr("Audio/Video Distribution Transport Protocol");
    case QBluetoothUuid::Cmtp: return QBluetoothServiceDiscoveryAgent::tr("Common ISDN Access Protocol");
    case QBluetoothUuid::UdiCPlain: return QBluetoothServiceDiscoveryAgent::tr("UdiCPlain");
    case QBluetoothUuid::McapControlChannel: return QBluetoothServiceDiscoveryAgent::tr("Multi-Channel Adaptation Protocol -Conrol");
    case QBluetoothUuid::McapDataChannel: return QBluetoothServiceDiscoveryAgent::tr("Multi-Channel Adaptation Protocol - Data");
    case QBluetoothUuid::L2cap: return QBluetoothServiceDiscoveryAgent::tr("Layer 2 Control Protocol");
    default:
        break;
    }

    return QString();
}

/*!
    Returns true if \a other is equal to this Bluetooth UUID, otherwise false.
*/
bool QBluetoothUuid::operator==(const QBluetoothUuid &other) const
{
    return QUuid::operator==(other);
}

QT_END_NAMESPACE
