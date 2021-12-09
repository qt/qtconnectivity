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

#include "qpcscslot_p.h"
#include "qpcscmanager_p.h"
#include "qpcsccard_p.h"
#include <QtCore/QLoggingCategory>

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(QT_NFC_PCSC)

QPcscSlot::QPcscSlot(const QPcscSlotName &name, QPcscManager *manager)
    : QObject(manager), m_name(name)
{
}

QPcscSlot::~QPcscSlot()
{
    qCDebug(QT_NFC_PCSC) << Q_FUNC_INFO << this;
    if (m_insertedCard)
        m_insertedCard->invalidate();
}

void QPcscSlot::processStateChange(DWORD eventId, bool createCards)
{
    qCDebug(QT_NFC_PCSC) << Q_FUNC_INFO;

    // Check if the currently inserted card is still valid
    if (!m_insertedCard.isNull()) {
        if (m_insertedCard->checkCardPresent())
            return;
        qCDebug(QT_NFC_PCSC) << "Removing card from slot" << m_name;
        m_insertedCard->invalidate();
        m_insertedCard.clear();
    }

    auto manager = qobject_cast<QPcscManager *>(parent());

    if (manager != nullptr && createCards
        && (eventId
            & (SCARD_STATE_PRESENT | SCARD_STATE_MUTE | SCARD_STATE_UNPOWERED
               | SCARD_STATE_EXCLUSIVE))
                == SCARD_STATE_PRESENT) {
        qCDebug(QT_NFC_PCSC) << "New card in slot" << m_name;

        m_insertedCard = manager->connectToCard(this);
    }
}

void QPcscSlot::invalidateInsertedCard()
{
    if (m_insertedCard)
        m_insertedCard->invalidate();
}

QT_END_NAMESPACE
