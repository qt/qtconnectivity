/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the Qt Toolkit.
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

#ifndef BOARD_H
#define BOARD_H

#include <QObject>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <QPixmap>
#include <QPropertyAnimation>

class Board : public QGraphicsView
{
    Q_OBJECT
    Q_PROPERTY(int connectRotation READ connectRotation WRITE setConnectRotation)
    Q_PROPERTY(qreal connectOpacity READ connectOpacity WRITE setConnectOpacity)
public:
    explicit Board(QWidget *parent = 0);

    enum Edge {
        Top = 1,
        Bottom,
        Left,
        Right
    };

    enum BoardSize {
        Width = 640,
        Height = 360,
        Paddle = 50
    };


    QGraphicsScene *getScene() {
        return scene;
    }

    int connectRotation(){
        return connectIcon->rotation();
    }

    qreal connectOpacity(){
        return connectIcon->opacity();
    }

    void setConnectRotation(int rot);
    void setConnectOpacity(qreal op);

signals:
    void ballCollision(Board::Edge pos);
    void scored(Board::Edge pos);

public slots:
    void setBall(int x, int y);
    void setLeftPaddle(int y);
    void setRightPaddle(int y);
    void setScore(int l, int r);

    void setStatus(QString text, int opacity_start, int opacity_end);

    void sceneChanged(const QList<QRectF> &region);

    void animateConnect(bool start = true);
    void fadeConnect(bool out = true);

private:
    QGraphicsScene *scene;
    QGraphicsRectItem *ball;
    QGraphicsRectItem *topWall;
    QGraphicsRectItem *bottomWall;
    QGraphicsRectItem *leftPaddle;
    QGraphicsRectItem *rightPaddle;

    QGraphicsTextItem *leftScore;
    QGraphicsTextItem *rightScore;

    QGraphicsTextItem *status;

    QPixmap icon;
    QGraphicsPixmapItem *connectIcon;
    QPropertyAnimation *connectAnimation;

    void checkBall(int x, int y);
};

#endif // BOARD_H
