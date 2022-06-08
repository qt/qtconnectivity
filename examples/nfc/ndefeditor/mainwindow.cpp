// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "textrecordeditor.h"
#include "urirecordeditor.h"
#include "mimeimagerecordeditor.h"

#include <QtNfc/qndefnfcurirecord.h>
#include <QtNfc/qndefnfctextrecord.h>
#include <QtNfc/qndefrecord.h>
#include <QtNfc/qndefmessage.h>
#include <QtNfc/qnearfieldmanager.h>
#include <QtNfc/qnearfieldtarget.h>

#include <QtWidgets/QMenu>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QFrame>
#include <QtWidgets/QLabel>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QScroller>
#include <QtWidgets/QApplication>
#include <QtGui/QScreen>

class EmptyRecordLabel : public QLabel
{
    Q_OBJECT

public:
    EmptyRecordLabel() : QLabel(tr("Empty Record")) { }
    ~EmptyRecordLabel() { }

    void setRecord(const QNdefRecord &record)
    {
        Q_UNUSED(record);
    }

    QNdefRecord record() const
    {
        return QNdefRecord();
    }
};

class UnknownRecordLabel : public QLabel
{
    Q_OBJECT

public:
    UnknownRecordLabel() : QLabel(tr("Unknown Record Type")) { }
    ~UnknownRecordLabel() { }

    void setRecord(const QNdefRecord &record) { m_record = record; }
    QNdefRecord record() const { return m_record; }

private:
    QNdefRecord m_record;
};

template<typename T>
void addRecord(Ui::MainWindow *ui, const QNdefRecord &record = QNdefRecord())
{
    QVBoxLayout *vbox = qobject_cast<QVBoxLayout *>(ui->scrollAreaWidgetContents->layout());
    if (!vbox)
        return;

    if (!vbox->isEmpty()) {
        QFrame *hline = new QFrame;
        hline->setFrameShape(QFrame::HLine);
        hline->setObjectName(QStringLiteral("line-spacer"));

        vbox->addWidget(hline);
    }

    T *recordEditor = new T;
    recordEditor->setObjectName(QStringLiteral("record-editor"));

    vbox->addWidget(recordEditor);

    if (!record.isEmpty())
        recordEditor->setRecord(record);
}

MainWindow::MainWindow(QWidget *parent)
:   QMainWindow(parent), ui(new Ui::MainWindow), m_touchAction(NoAction)
{
    ui->setupUi(this);

    connect(ui->addRecord, &QPushButton::clicked, this, &MainWindow::showMenu);

    QVBoxLayout *vbox = new QVBoxLayout;
    ui->scrollAreaWidgetContents->setLayout(vbox);
#if (defined(Q_OS_ANDROID) && !defined(Q_OS_ANDROID_EMBEDDED)) || defined(Q_OS_IOS)
    QScroller::grabGesture(ui->scrollArea, QScroller::TouchGesture);
    ui->scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
#endif

    // Detect keyboard show/hide. We can't directly update the UI to ensure
    // that the focused widget is active. Instead we wait for a resizeEvent
    // that happens shortly after the keyboard is shown, and do all the
    // processing there.
    QInputMethod *inputMethod = qApp->inputMethod();
    connect(inputMethod, &QInputMethod::visibleChanged,
            [this, inputMethod]() { m_keyboardVisible = inputMethod->isVisible(); });

    //! [QNearFieldManager init]
    m_manager = new QNearFieldManager(this);
    connect(m_manager, &QNearFieldManager::targetDetected,
            this, &MainWindow::targetDetected);
    connect(m_manager, &QNearFieldManager::targetLost,
            this, &MainWindow::targetLost);
    //! [QNearFieldManager init]
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::resizeEvent(QResizeEvent *e)
{
    QMainWindow::resizeEvent(e);
    if (m_keyboardVisible) {
        QWidget *areaWidget = ui->scrollAreaWidgetContents;
        QList<QWidget *> childWidgets = areaWidget->findChildren<QWidget *>();
        for (const auto widget : childWidgets) {
            if (widget->hasFocus()) {
                ui->scrollArea->ensureWidgetVisible(widget);
            }
        }
    }
}

void MainWindow::addNfcTextRecord()
{
    addRecord<TextRecordEditor>(ui);
}

void MainWindow::addNfcUriRecord()
{
    addRecord<UriRecordEditor>(ui);
}

void MainWindow::addMimeImageRecord()
{
    addRecord<MimeImageRecordEditor>(ui);
}

void MainWindow::addEmptyRecord()
{
    addRecord<EmptyRecordLabel>(ui);
}

void MainWindow::loadMessage()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Select NDEF Message"));
    if (filename.isEmpty())
        return;

    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly))
        return;

    QByteArray ndef = file.readAll();

    ndefMessageRead(QNdefMessage::fromByteArray(ndef));

    file.close();
}

void MainWindow::saveMessage()
{
    QString filename = QFileDialog::getSaveFileName(this, tr("Select NDEF Message"));
    if (filename.isEmpty())
        return;

    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly))
        return;

    file.write(ndefMessage().toByteArray());

    file.close();
}

void MainWindow::touchReceive()
{
    ui->status->setStyleSheet(QStringLiteral("background: blue"));

    m_touchAction = ReadNdef;

    //! [QNearFieldManager start detection]
    m_manager->startTargetDetection(QNearFieldTarget::NdefAccess);
    //! [QNearFieldManager start detection]
}

void MainWindow::touchStore()
{
    ui->status->setStyleSheet(QStringLiteral("background: yellow"));

    m_touchAction = WriteNdef;

    m_manager->startTargetDetection(QNearFieldTarget::NdefAccess);
}

//! [QNearFieldTarget detected]
void MainWindow::targetDetected(QNearFieldTarget *target)
{
    switch (m_touchAction) {
    case NoAction:
        break;
    case ReadNdef:
        connect(target, &QNearFieldTarget::ndefMessageRead, this, &MainWindow::ndefMessageRead);
        connect(target, &QNearFieldTarget::error, this, &MainWindow::targetError);

        m_request = target->readNdefMessages();
        if (!m_request.isValid()) // cannot read messages
            targetError(QNearFieldTarget::NdefReadError, m_request);
        break;
    case WriteNdef:
        connect(target, &QNearFieldTarget::requestCompleted, this, &MainWindow::ndefMessageWritten);
        connect(target, &QNearFieldTarget::error, this, &MainWindow::targetError);

        m_request = target->writeNdefMessages(QList<QNdefMessage>() << ndefMessage());
        if (!m_request.isValid()) // cannot write messages
            targetError(QNearFieldTarget::NdefWriteError, m_request);
        break;
    }
}
//! [QNearFieldTarget detected]

//! [QNearFieldTarget lost]
void MainWindow::targetLost(QNearFieldTarget *target)
{
    target->deleteLater();
}
//! [QNearFieldTarget lost]

void MainWindow::ndefMessageRead(const QNdefMessage &message)
{
    clearMessage();

    for (const QNdefRecord &record : message) {
        if (record.isRecordType<QNdefNfcTextRecord>()) {
            addRecord<TextRecordEditor>(ui, record);
        } else if (record.isRecordType<QNdefNfcUriRecord>()) {
            addRecord<UriRecordEditor>(ui, record);
        } else if (record.typeNameFormat() == QNdefRecord::Mime &&
                   record.type().startsWith("image/")) {
            addRecord<MimeImageRecordEditor>(ui, record);
        } else if (record.isEmpty()) {
            addRecord<EmptyRecordLabel>(ui);
        } else {
            addRecord<UnknownRecordLabel>(ui, record);
        }
    }

    ui->status->setStyleSheet(QString());
    //! [QNearFieldManager stop detection]
    m_manager->stopTargetDetection();
    //! [QNearFieldManager stop detection]
    m_request = QNearFieldTarget::RequestId();
    ui->statusBar->clearMessage();
}

void MainWindow::ndefMessageWritten(const QNearFieldTarget::RequestId &id)
{
    if (id == m_request) {
        ui->status->setStyleSheet(QString());
        m_manager->stopTargetDetection();
        m_request = QNearFieldTarget::RequestId();
        ui->statusBar->clearMessage();
    }
}

void MainWindow::targetError(QNearFieldTarget::Error error, const QNearFieldTarget::RequestId &id)
{
    Q_UNUSED(error);
    Q_UNUSED(id);

    if (m_request == id) {
        switch (error) {
        case QNearFieldTarget::NoError:
            ui->statusBar->clearMessage();
            break;
        case QNearFieldTarget::UnsupportedError:
            ui->statusBar->showMessage(tr("Unsupported tag"));
            break;
        case QNearFieldTarget::TargetOutOfRangeError:
            ui->statusBar->showMessage(tr("Tag removed from field"));
            break;
        case QNearFieldTarget::NoResponseError:
            ui->statusBar->showMessage(tr("No response from tag"));
            break;
        case QNearFieldTarget::ChecksumMismatchError:
            ui->statusBar->showMessage(tr("Checksum mismatch"));
            break;
        case QNearFieldTarget::InvalidParametersError:
            ui->statusBar->showMessage(tr("Invalid parameters"));
            break;
        case QNearFieldTarget::NdefReadError:
            ui->statusBar->showMessage(tr("NDEF read error"));
            break;
        case QNearFieldTarget::NdefWriteError:
            ui->statusBar->showMessage(tr("NDEF write error"));
            break;
        default:
            ui->statusBar->showMessage(tr("Unknown error"));
        }

        ui->status->setStyleSheet(QString());
        m_manager->stopTargetDetection();
        m_request = QNearFieldTarget::RequestId();
    }
}

void MainWindow::showMenu()
{
    // We have to manually call QMenu::popup() because of QTBUG-98651.
    // And we need to re-create menu each time because of QTBUG-97482.
    if (m_menu) {
        m_menu->setParent(nullptr);
        delete m_menu;
    }
    m_menu = new QMenu(this);
    m_menu->addAction(tr("NFC Text Record"), this, &MainWindow::addNfcTextRecord);
    m_menu->addAction(tr("NFC URI Record"), this, &MainWindow::addNfcUriRecord);
    m_menu->addAction(tr("MIME Image Record"), this, &MainWindow::addMimeImageRecord);
    m_menu->addAction(tr("Empty Record"), this, &MainWindow::addEmptyRecord);

    // Use menu's sizeHint() to position it so that its right side is aligned
    // with button's right side.
    QPushButton *button = ui->addRecord;
    const int x = button->x() + button->width() - m_menu->sizeHint().width();
    const int y = button->y() + button->height();
    m_menu->popup(mapToGlobal(QPoint(x, y)));
}

void MainWindow::clearMessage()
{
    QWidget *scrollArea = ui->scrollAreaWidgetContents;

    qDeleteAll(scrollArea->findChildren<QWidget *>(QStringLiteral("line-spacer")));
    qDeleteAll(scrollArea->findChildren<QWidget *>(QStringLiteral("record-editor")));
}

QNdefMessage MainWindow::ndefMessage() const
{
    QVBoxLayout *vbox = qobject_cast<QVBoxLayout *>(ui->scrollAreaWidgetContents->layout());
    if (!vbox)
        return QNdefMessage();

    QNdefMessage message;

    for (int i = 0; i < vbox->count(); ++i) {
        QWidget *widget = vbox->itemAt(i)->widget();

        if (TextRecordEditor *editor = qobject_cast<TextRecordEditor *>(widget)) {
            message.append(editor->record());
        } else if (UriRecordEditor *editor = qobject_cast<UriRecordEditor *>(widget)) {
            message.append(editor->record());
        } else if (MimeImageRecordEditor *editor = qobject_cast<MimeImageRecordEditor *>(widget)) {
            message.append(editor->record());
        } else if (qobject_cast<EmptyRecordLabel *>(widget)) {
            message.append(QNdefRecord());
        } else if (UnknownRecordLabel *label = qobject_cast<UnknownRecordLabel *>(widget)) {
            message.append(label->record());
        }
    }

    return message;
}

#include "mainwindow.moc"
