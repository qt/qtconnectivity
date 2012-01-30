/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/
**
** This file is part of the Qt Toolkit.
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

#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <QObject>
#include <QTimer>
#include <QTime>
#include "board.h"

class Controller : public QObject
{
    Q_OBJECT
public:
    explicit Controller(QObject *parent = 0);

signals:
    void moveBall(int x, int y);
    void score(int left, int right);
    void fps(const QString &str);

public slots:
    void ballCollision(Board::Edge pos);
    void scored(Board::Edge pos);
    void resetBoard();
    void refresh();

    void moveLeftPaddle(int y);
    void moveRightPaddle(int y);

    void tick();

    void start();
    void stop();

private:
    QTimer *timer;
    QTime *elapsed;
    QTime *fpscntr;
    int frames;
    int ball_x;
    int speed_x;
    int ball_y;
    int speed_y;
    int score_left;
    int score_right;
    int col_x;
    int col_y;

    int rightPaddleForce;
    int leftPaddleForce;
    int rightPaddleLast;
    int leftPaddleLast;
    int rightPowerUp;
    int leftPowerUp;

    void restart_ball();

};

#endif // CONTROLLER_H
