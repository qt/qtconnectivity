/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtBluetooth module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL21$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
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
** As a special exception, The Qt Company gives you certain additional
** rights. These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QLEADVERTISER_P_H
#define QLEADVERTISER_P_H

#include "qlowenergyadvertisingdata.h"
#include "qlowenergyadvertisingparameters.h"

#ifdef QT_BLUEZ_BLUETOOTH
#include "bluez/bluez_data_p.h"
#endif

#include <QtCore/qobject.h>
#include <QtCore/qvector.h>

QT_BEGIN_NAMESPACE

class QLeAdvertiser : public QObject
{
    Q_OBJECT
public:
    void startAdvertising() { doStartAdvertising(); }
    void stopAdvertising() { doStopAdvertising(); }

signals:
    void errorOccurred();

protected:
    QLeAdvertiser(const QLowEnergyAdvertisingParameters &params,
                  const QLowEnergyAdvertisingData &advData,
                  const QLowEnergyAdvertisingData &responseData, QObject *parent)
        : QObject(parent), m_params(params), m_advData(advData), m_responseData(responseData) {}
    virtual ~QLeAdvertiser() { }

    const QLowEnergyAdvertisingParameters &parameters() const { return m_params; }
    const QLowEnergyAdvertisingData &advertisingData() const { return m_advData; }
    const QLowEnergyAdvertisingData &scanResponseData() const { return m_responseData; }

private:
    virtual void doStartAdvertising() = 0;
    virtual void doStopAdvertising() = 0;

    const QLowEnergyAdvertisingParameters m_params;
    const QLowEnergyAdvertisingData m_advData;
    const QLowEnergyAdvertisingData m_responseData;
};


#ifdef QT_BLUEZ_BLUETOOTH
struct AdvData;
struct AdvParams;
class HciManager;

class QLeAdvertiserBluez : public QLeAdvertiser
{
public:
    QLeAdvertiserBluez(const QLowEnergyAdvertisingParameters &params,
                       const QLowEnergyAdvertisingData &advertisingData,
                       const QLowEnergyAdvertisingData &scanResponseData, HciManager &hciManager,
                       QObject *parent = nullptr);
    ~QLeAdvertiserBluez();

private:
    void doStartAdvertising() override;
    void doStopAdvertising() override;

    void setPowerLevel(AdvData &advData);
    void setFlags(AdvData &advData);
    void setServicesData(const QLowEnergyAdvertisingData &src, AdvData &dest);
    void setManufacturerData(const QLowEnergyAdvertisingData &src, AdvData &dest);
    void setLocalNameData(const QLowEnergyAdvertisingData &src, AdvData &dest);

    void queueCommand(OpCodeCommandField ocf, const QByteArray &advertisingData);
    void sendNextCommand();
    void queueAdvertisingCommands();
    void queueReadTxPowerLevelCommand();
    void toggleAdvertising(bool enable);
    void setAdvertisingParams();
    void setAdvertisingInterval(AdvParams &params);
    void setData(bool isScanResponseData);
    void setAdvertisingData();
    void setScanResponseData();
    void setWhiteList();

    void handleCommandCompleted(quint16 opCode, quint8 status, const QByteArray &advertisingData);
    void handleError();

    HciManager &m_hciManager;

    struct Command {
        Command() {}
        Command(OpCodeCommandField ocf, const QByteArray &data) : ocf(ocf), data(data) { }
        OpCodeCommandField ocf;
        QByteArray data;
    };
    QVector<Command> m_pendingCommands;

    quint8 m_powerLevel;
    bool m_sendPowerLevel;
    bool m_disableCommandFinished;
};
#endif // QT_BLUEZ_BLUETOOTH

QT_END_NAMESPACE

#endif // Include guard.
