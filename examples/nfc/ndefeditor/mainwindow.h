// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtNfc/qnearfieldtarget.h>

#include <QtWidgets/QMainWindow>

QT_FORWARD_DECLARE_CLASS(QNearFieldManager)
QT_FORWARD_DECLARE_CLASS(QNdefMessage)
QT_FORWARD_DECLARE_CLASS(QScreen)
QT_FORWARD_DECLARE_CLASS(QMenu)

QT_BEGIN_NAMESPACE
namespace Ui {
    class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void resizeEvent(QResizeEvent *e) override;

private slots:
    //! [0]
    void addNfcTextRecord();
    void addNfcUriRecord();
    void addMimeImageRecord();
    void addEmptyRecord();
    //! [0]
    void clearMessage();

    void loadMessage();
    void saveMessage();

    void touchReceive();
    void touchStore();

    void targetDetected(QNearFieldTarget *target);
    void targetLost(QNearFieldTarget *target);

    void ndefMessageRead(const QNdefMessage &message);
    void ndefMessageWritten(const QNearFieldTarget::RequestId &id);
    void targetError(QNearFieldTarget::Error error, const QNearFieldTarget::RequestId &id);

    void showMenu();

private:
    enum TouchAction {
        NoAction,
        ReadNdef,
        WriteNdef
    };

    QNdefMessage ndefMessage() const;
    void handleScreenChange();
    void updateWidgetLayout(Qt::ScreenOrientation orientation);

private:
    Ui::MainWindow *ui;
    QMenu *m_menu = nullptr;

    QNearFieldManager *m_manager;
    TouchAction m_touchAction;
    QNearFieldTarget::RequestId m_request;
    bool m_keyboardVisible = false;
    QScreen *m_screen = nullptr;
};

#endif // MAINWINDOW_H
