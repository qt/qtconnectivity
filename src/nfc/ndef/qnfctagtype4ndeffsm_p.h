/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtNfc module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QNFCTAGTYPE4NDEFFSM_P_H
#define QNFCTAGTYPE4NDEFFSM_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qndefaccessfsm_p.h"
#include "qapduutils_p.h"

QT_BEGIN_NAMESPACE

class QNfcTagType4NdefFsm : public QNdefAccessFsm
{
public:
    QByteArray getCommand(Action &nextAction) override;
    QNdefMessage getMessage(Action &nextAction) override;
    Action provideResponse(const QByteArray &response) override;

    Action detectNdefSupport() override;
    Action readMessages() override;
    Action writeMessages(const QList<QNdefMessage> &messages) override;

private:
    enum State {
        SelectApplicationForProbe,
        SelectCCFile,
        ReadCCFile,
        NdefSupportDetected,
        NdefNotSupported,

        SelectApplicationForRead,
        SelectNdefFileForRead,
        ReadNdefMessageLength,
        ReadNdefMessage,
        NdefMessageRead,

        SelectApplicationForWrite,
        SelectNdefFileForWrite,
        ClearNdefLength,
        WriteNdefFile,
        WriteNdefLength,
        NdefMessageWritten // Only for target state, it is never actually reached
    };

    State m_currentState = SelectApplicationForProbe;
    State m_targetState = SelectApplicationForProbe;

    // Initialized during the detection phase
    uint16_t m_maxReadSize;
    uint16_t m_maxUpdateSize;
    QByteArray m_ndefFileId;
    uint16_t m_maxNdefSize = 0xFFFF;
    bool m_writable;

    // Used during the read and write operations
    uint16_t m_fileSize;
    uint16_t m_fileOffset;
    QByteArray m_ndefData;

    Action handleSimpleResponse(const QResponseApdu &response, State okState, State failedState,
                                Action okAction = SendCommand);

    Action handleReadCCResponse(const QResponseApdu &response);
    Action handleReadFileLengthResponse(const QResponseApdu &response);
    Action handleReadFileResponse(const QResponseApdu &response);
    Action handleWriteNdefFileResponse(const QResponseApdu &response);
};

QT_END_NAMESPACE

#endif // QNFCTAGTYPE4NDEFFSM_P_H
