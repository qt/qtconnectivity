/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtNfc module.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
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
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
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

#include "mainwindow.h"

#include <QUrl>
#include <QDesktopServices>
#include <QLabel>
#include <QLayout>
#include <QScreen>

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent),
      m_titleLabel(new QLabel(this)),
      m_infoLabel(new QLabel(tr("Enable NFC"), this)),
      m_uriLabel(new QLabel(this)),
      m_landscapeIconLabel(new QLabel(this)),
      m_portraitIconLabel(new QLabel(this))
{
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_infoLabel->setAlignment(Qt::AlignCenter);
    m_uriLabel->setAlignment(Qt::AlignCenter);
    m_landscapeIconLabel->setAlignment(Qt::AlignCenter);
    m_portraitIconLabel->setAlignment(Qt::AlignCenter);

    QFont f = m_titleLabel->font();
    f.setBold(true);
    m_titleLabel->setFont(f);

    QVBoxLayout *verticalLayout = new QVBoxLayout;
    verticalLayout->addWidget(m_titleLabel);
    verticalLayout->addWidget(m_infoLabel);
    verticalLayout->addWidget(m_portraitIconLabel);
    verticalLayout->addWidget(m_uriLabel);

    QHBoxLayout *horizontalLayout = new QHBoxLayout;
    horizontalLayout->addWidget(m_landscapeIconLabel);
    horizontalLayout->addLayout(verticalLayout);
    horizontalLayout->setSpacing(0);

    setLayout(horizontalLayout);

    handleScreenChange();
}

MainWindow::~MainWindow()
{
}

void MainWindow::displayAnnotatedUrl(const QUrl &url, const QString &title, const QPixmap &pixmap)
{
    m_infoLabel->setHidden(true);

    m_uriLabel->setText(url.toString());
    m_titleLabel->setText(title);

    m_pixmap = pixmap;
    updateWidgetLayout(m_screen->orientation());
}

void MainWindow::nfcStateChanged(bool enabled)
{
    const QString text = enabled ? "Touch a tag" : "Enable NFC";
    m_infoLabel->setText(text);
}

void MainWindow::showTagError(const QString &message)
{
    m_uriLabel->clear();
    m_titleLabel->clear();
    m_pixmap = QPixmap();
    m_landscapeIconLabel->setVisible(false);
    m_portraitIconLabel->setVisible(false);

    m_infoLabel->setHidden(false);
    m_infoLabel->setText(message);
}

void MainWindow::mouseReleaseEvent(QMouseEvent * /*event*/)
{
    QDesktopServices::openUrl(QUrl(m_uriLabel->text()));
}

void MainWindow::moveEvent(QMoveEvent * /*event*/)
{
    if (screen() != m_screen)
        handleScreenChange();
}

void MainWindow::updateWidgetLayout(Qt::ScreenOrientation orientation)
{
    m_landscapeIconLabel->setVisible(false);
    m_portraitIconLabel->setVisible(false);
    if (!m_pixmap.isNull()) {
        const bool landscapeMode = (orientation == Qt::LandscapeOrientation)
                || (orientation == Qt::InvertedLandscapeOrientation);
        QLabel *imageLabel = landscapeMode ? m_landscapeIconLabel : m_portraitIconLabel;
        imageLabel->setVisible(true);
        if (m_pixmap.width() > imageLabel->width() || m_pixmap.height() > imageLabel->height())
            imageLabel->setPixmap(m_pixmap.scaled(imageLabel->size(), Qt::KeepAspectRatio));
        else
            imageLabel->setPixmap(m_pixmap);
    }
}

void MainWindow::handleScreenChange()
{
    if (m_screen)
        disconnect(m_screen, &QScreen::orientationChanged, this, &MainWindow::updateWidgetLayout);

    m_screen = screen();
    connect(m_screen, &QScreen::orientationChanged, this, &MainWindow::updateWidgetLayout);

    updateWidgetLayout(m_screen->orientation());
}
