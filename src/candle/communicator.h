#ifndef COMMUNICATOR_H
#define COMMUNICATOR_H

#include <QScrollBar>
//#include "frmmain.h"
#include "globals.h"
#include "connection/connection.h"
#include "machine/settings.h"
#include "ui_frmmain.h"

#ifdef WINDOWS
    #include <QtWinExtras/QtWinExtras>
#endif

namespace Ui {
class frmMain;
}

class Communicator : public QObject
{
    friend class frmMain;

    Q_OBJECT

public:
    Communicator(
        Connection *connection,
        Ui::frmMain *ui,
        //frmMain *form,
        QObject *parent);
    SendCommandResult sendCommand(QString command, int tableIndex = -1, bool showInConsole = true, bool wait = false);
    void sendRealtimeCommand(QString command);
    void sendCommands(QString commands, int tableIndex = -1);
    void clearCommandsAndQueue();
    void clearQueue();
    void reset();
private:
    Connection *m_connection;
    Settings *m_settings;
    QList<CommandAttributes> m_commands;
    QList<CommandQueue> m_queue;
    SenderState m_senderState;
    DeviceState m_deviceState;
    QTime m_startTime;

    bool m_reseting;
    bool m_resetCompleted;
    bool m_aborting;
    bool m_statusReceived;
    bool m_homing;
    Ui::frmMain *ui;

    void onSerialPortReadyRead(QString);
    bool dataIsReset(QString);
    void setSenderState(SenderState);
    void setDeviceState(DeviceState);
signals:
    void responseReceived(QString command, int tableIndex, QString response);
    void statusReceived(QString status);
    void senderStateChanged(int state);
    void deviceStateChanged(int state);
};

#endif // COMMUNICATOR_H
