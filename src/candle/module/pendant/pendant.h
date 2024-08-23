// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef PENDANT_H
#define PENDANT_H

#include <QObject>
#include <QTcpServer>

class Pendant : public QObject
{
        Q_OBJECT
    public:
        explicit Pendant(QObject *parent = nullptr);

    private:
        QTcpServer *server;

    signals:
};

#endif // PENDANT_H
