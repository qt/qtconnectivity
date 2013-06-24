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

#include "controller.h"

#include <QDateTime>
#include <QDebug>

#define FRAME_RATE 60

Controller::Controller(QObject *parent) :
    QObject(parent), timer(new QTimer), elapsed(new QTime), score_left(0), score_right(0),
    col_x(0), col_y(0),
    rightPaddleForce(0), leftPaddleForce(0),
    rightPaddleLast(0), leftPaddleLast(0),
    rightPowerUp(0), leftPowerUp(0)
{

    qsrand(QDateTime::currentDateTime().time().msec());


    timer->setInterval(1000/FRAME_RATE);
//    timer->setInterval(0);
    connect(timer, SIGNAL(timeout()), SLOT(tick()));
    srand(QDateTime::currentDateTime().time().msec());

    fpscntr = new QTime;
    frames = 0;

    timer->start();
    restart_ball();
}

void Controller::tick()
{
    if (col_x)
        col_x--;
    if (col_y)
        col_y--;

//    frames++;
//    if (fpscntr->elapsed() > 1000){
//        int f = frames/(fpscntr->elapsed()/1000.0);
//        emit fps(QString("FPS %1").arg(f));
//        frames = 0;
//        fpscntr->restart();
//    }

    int msec = elapsed->elapsed();
    elapsed->restart();

    ball_x += speed_x*msec/1000;
    ball_y += speed_y*msec/1000;


    const int decay = 5;

    if (leftPaddleForce > decay)
        leftPaddleForce-=decay;
    else if (leftPaddleForce < -decay)
        leftPaddleForce+=decay;
    if (rightPaddleForce > decay)
        rightPaddleForce-=decay;
    else if (rightPaddleForce < -decay)
        rightPaddleForce+=decay;

    if (rightPaddleForce <= decay)
        rightPowerUp++;

    if (leftPaddleForce <= decay)
        leftPowerUp++;


//    ttf++;
//    if (msec > 1000/FRAME_RATE+2 || msec < 1000/FRAME_RATE-2)
//        dev++;

//    if (!(i++%120)) {
//        qDebug() << "powerUp: " << leftPowerUp << rightPowerUp << leftPaddleForce << rightPaddleForce << speed_x*msec/1000 << speed_y*msec/1000 << msec << dev;
//        ttf = dev =0;
//    }

    emit moveBall(ball_x, ball_y);
}


static inline int paddle_boost(int force){
    if (force > 30)
        return -3*FRAME_RATE;
    else if (force > 20)
        return -2*FRAME_RATE;
    else if (force > 6)
        return -1*FRAME_RATE;
    else if (force < -30)
        return 3*FRAME_RATE;
    else if (force < -20)
        return 2*FRAME_RATE;
    else if (force < -6)
        return 1*FRAME_RATE;
    return 0;
}

void Controller::ballCollision(Board::Edge pos)
{

    if ((pos == Board::Top || pos == Board::Bottom) && !col_y) {
        speed_y *= -1;
        col_y = 10;
    }

    if (pos == Board::Left && !col_x) {
        speed_x *= -1;
        speed_y += paddle_boost(leftPaddleForce);
        col_x = 10;

        if (leftPowerUp > 75 && speed_x < 8*FRAME_RATE){
            speed_x *= 2;
            leftPowerUp = 0;
        }
    }
    else if (pos == Board::Right && !col_x) {
        speed_x *= -1;
        speed_y += paddle_boost(rightPaddleForce);
        col_x = 10;

        if (rightPowerUp > 75 && speed_x > -8*FRAME_RATE){
            speed_x *= 2;
            rightPowerUp = 0;
        }

    }
//    tick();
//    QMetaObject::invokeMethod(this, "moveBall", Qt::QueuedConnection, Q_ARG(int, ball_x), Q_ARG(int, ball_y));
//    emit moveBall(ball_x, ball_y);

}

void Controller::scored(Board::Edge pos)
{
    if (!timer->isActive())
        return;

    if (pos == Board::Left)
        emit score(score_left, ++score_right);
    else if (pos == Board::Right)
        emit score(++score_left, score_right);

    restart_ball();
}

void Controller::restart_ball()
{
    if (!timer->isActive())
        return;

    elapsed->start();

    ball_x = Board::Width/2;
    ball_y = Board::Height/2;

//    ball_y = (qrand()%(Board::Height/2))+Board::Height/4;
    ball_y = (qrand()%(Board::Height-48))+24;

    // Speed in in pixels/second

    const int max = 4*FRAME_RATE;
    const int min_x = 2*FRAME_RATE;
    const int min_y = 1.5*FRAME_RATE;


    speed_y = min_y+qrand()%(max-min_y);
    if (speed_y%2)
        speed_y *= -1;


    speed_x = min_x+qrand()%(max-min_y);
    if (speed_x%2)
        speed_x *= -1;

    leftPowerUp = rightPowerUp = 0;

    emit moveBall(ball_x, ball_y);
}


void Controller::resetBoard()
{
    if (!timer->isActive())
        return;

    score_left = score_right = 0;
    restart_ball();
}

void Controller::stop()
{
    timer->stop();
}

void Controller::start()
{
    timer->start();
    fpscntr->restart();
    frames = 0;
}

void Controller::moveLeftPaddle(int y)
{
    leftPaddleForce += leftPaddleLast-y;
    leftPaddleLast = y;
    leftPowerUp = 0;
}

void Controller::moveRightPaddle(int y)
{
    rightPaddleForce += rightPaddleLast-y;
    rightPaddleLast =y;
    rightPowerUp = 0;
}

void Controller::refresh()
{
    emit moveBall(ball_x, ball_y);
    emit score(score_left, score_right);
}
