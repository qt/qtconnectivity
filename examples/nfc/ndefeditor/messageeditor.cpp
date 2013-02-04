/****************************************************************************
**
** Copyright (C) 2013 Aaron McCarthy <mccarthy.aaron@gmail.com>
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtNfc module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "ui_messageeditor.h"
#include "messageeditor.h"
#include "recordeditor.h"

#include <QtWidgets/QMenu>

MessageEditor::MessageEditor(QWidget *parent)
:   QWidget(parent), ui(new Ui::MessageEditor), m_menu(0)
{
    ui->setupUi(this);
}

MessageEditor::~MessageEditor()
{
    delete ui;
}

QMenu *MessageEditor::actionMenu() const
{
    return m_menu;
}

void MessageEditor::addAction(const QString &title, QObject *receiver, const char *slot)
{
    if (!m_menu)
        m_menu = new QMenu(this);

    m_menu->addAction(title, receiver, slot);
}

void MessageEditor::addRecordEditor(RecordEditor *recordEditor, const QNdefRecord &record)
{
    QVBoxLayout *vbox = qobject_cast<QVBoxLayout *>(layout());
    if (!vbox)
        return;

    if (!vbox->isEmpty()) {
        QFrame *hline = new QFrame;
        hline->setFrameShape(QFrame::HLine);
        hline->setObjectName(QLatin1String("line-spacer"));

        vbox->addWidget(hline);
    }

    recordEditor->setObjectName(QLatin1String("record-editor"));

    if (!record.isEmpty())
        recordEditor->setRecord(record);

    vbox->addWidget(recordEditor);
}

QNdefMessage MessageEditor::ndefMessage() const
{
    QNdefMessage message;

    foreach (QObject *child, children()) {
        RecordEditor *editor = qobject_cast<RecordEditor *>(child);
        if (!editor)
            continue;

        message.append(editor->record());
    }

    return message;
}

void MessageEditor::clearRecords()
{
    foreach (QObject *child, children()) {
        if (child->objectName() == QLatin1String("line-spacer") ||
            child->objectName() == QLatin1String("record-editor")) {
            delete child;
        }
    }
}
