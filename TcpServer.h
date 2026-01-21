#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>

#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include "ScaleInterface232.h"
// #include <QObject>
#include <QQmlEngine>

class TcpServer : public QObject
{
    Q_OBJECT
    // QML_ELEMENT
public:
    explicit TcpServer(QObject *parent = nullptr);

    Q_INVOKABLE void startServer(int port);
    Q_INVOKABLE void stopServer();
    Q_INVOKABLE void sendData(const QString &data);
    Q_INVOKABLE void startInfiniteSend(const QString &data,
                                       int interval = 1000); // interval in milliseconds
    Q_INVOKABLE void stopInfiniteSend();
    Q_INVOKABLE void sendRespDonePrinter();
    Q_INVOKABLE void sendRespWeightTest();
    Q_INVOKABLE void applySerialParams(const QString &portName, int idx);
    Q_INVOKABLE void sendFrameFake1(int ms, const QString &w);
    Q_INVOKABLE void stopFake1();
    Q_INVOKABLE void sendFrameFake2(int ms, const QString &w);
    Q_INVOKABLE void stopFake2();
    Q_INVOKABLE void sendFrameFake3(int ms, const QString &w);
    Q_INVOKABLE void stopFake3();

signals:
    void clientConnected(QString address, int port);
    void clientDisconnected();
    void dataReceived(QString data);

private:
    QTcpServer *server;
    QList<QTcpSocket *> clients;
    QTimer *sendTimer;      // Timer for infinite sending
    QString sendDataString; // Data to send repeatedly
    // QTimer *responseSending;
    QTimer *timer1;
    QTimer *timer2;
    QTimer *timer3;
    QByteArray frame1;
    QByteArray frame;
    QByteArray frame2;

    QSerialPort *serialPort;
    QPointer<ScaleInterface232> m_scales232[3] = {nullptr, nullptr, nullptr};

    void sendZeroData(int u);
    void hookSignals(int idx);

private slots:
    void onNewConnection();
    void onReadyRead();
    void onClientDisconnected();
    void sendDataTimerTriggered();
};

#endif // TCPSERVER_H
