// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef MIMEIMAGERECORDEDITOR_H
#define MIMEIMAGERECORDEDITOR_H

#include <QtNfc/qndefrecord.h>

#include <QtWidgets/QWidget>

QT_USE_NAMESPACE

QT_BEGIN_NAMESPACE
namespace Ui {
    class MimeImageRecordEditor;
}
QT_END_NAMESPACE

//! [0]
class MimeImageRecordEditor : public QWidget
{
    Q_OBJECT

public:
    explicit MimeImageRecordEditor(QWidget *parent = 0);
    ~MimeImageRecordEditor();

    void setRecord(const QNdefRecord &record);
    QNdefRecord record() const;

public slots:
    void handleScreenOrientationChange(Qt::ScreenOrientation orientation);

protected:
    void resizeEvent(QResizeEvent *) override;

private:
    void updatePixmap();

    Ui::MimeImageRecordEditor *ui;
    QNdefRecord m_record;
    QPixmap m_pixmap;
    bool m_imageSelected = false;
    bool m_screenRotated = false;

private slots:
    void on_mimeImageOpen_clicked();
};
//! [0]
#endif // MIMEIMAGERECORDEDITOR_H
