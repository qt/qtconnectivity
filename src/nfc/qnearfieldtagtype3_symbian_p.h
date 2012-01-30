/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/
**
** This file is part of the QtNfc module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QNEARFIELDTAGTYPE3SYMBIAN_H
#define QNEARFIELDTAGTYPE3SYMBIAN_H

#include <qnearfieldtagtype3.h>
#include "symbian/nearfieldtagimpl_symbian.h"
#include "symbian/nearfieldndeftarget_symbian.h"

QT_BEGIN_HEADER

QTNFC_BEGIN_NAMESPACE

class QNearFieldTagType3Symbian : public QNearFieldTagType3, private QNearFieldTagImpl<QNearFieldTagType3Symbian>
{
    Q_OBJECT
public:

    explicit QNearFieldTagType3Symbian(CNearFieldNdefTarget *tag, QObject *parent = 0);

    ~QNearFieldTagType3Symbian();

    virtual QByteArray uid() const;

    void setAccessMethods(const QNearFieldTarget::AccessMethods& accessMethods)
    {
        _setAccessMethods(accessMethods);
    }

    QNearFieldTarget::AccessMethods accessMethods() const
    {
        return _accessMethods();
    }

    bool hasNdefMessage();
    RequestId readNdefMessages();
    RequestId writeNdefMessages(const QList<QNdefMessage> &messages);

#if 0
    quint16 systemCode();
    QList<quint16> services();
    int serviceMemorySize(quint16 serviceCode);


    RequestId serviceData(quint16 serviceCode);
    RequestId writeServiceData(quint16 serviceCode, const QByteArray &data);
#endif

    RequestId check(const QMap<quint16, QList<quint16> > &serviceBlockList);
    RequestId update(const QMap<quint16, QList<quint16> > &serviceBlockList,
                                 const QByteArray &data);

    bool isProcessingCommand() const { return _isProcessingRequest(); }
    RequestId sendCommand(const QByteArray &command);
    RequestId sendCommands(const QList<QByteArray> &commands);
    bool waitForRequestCompleted(const RequestId &id, int msecs = 5000);
private:
    const QByteArray& getIDm();
    QByteArray serviceBlockList2CmdParam(const QMap<quint16, QList<quint16> > &serviceBlockList, quint8& numberOfBlocks, bool isCheckCommand);
    QMap<quint16, QList<quint16> > cmd2ServiceBlockList(const QByteArray& cmd);

    QMap<quint16, QByteArray> checkResponse2ServiceBlockList(const QMap<quint16, QList<quint16> > &serviceBlockList, const QByteArray& response);

    void handleTagOperationResponse(const RequestId &id, const QByteArray &command, const QByteArray &response, bool emitRequestCompleted);
    QVariant decodeResponse(const QByteArray & command, const QByteArray &response);
private:
    QByteArray mIDm;
    friend class QNearFieldTagImpl<QNearFieldTagType3Symbian>;
};

QTNFC_END_NAMESPACE

typedef QMap<quint16,QByteArray> checkResponseType;
Q_DECLARE_METATYPE(checkResponseType)
QT_END_HEADER
#endif // QNEARFIELDTAGTYPE3SYMBIAN_H

