#include "TcpServer.h"
#include <QDebug>

TcpServer::TcpServer(QObject *parent)
    : QObject(parent)
    , server(new QTcpServer(this))
    , sendTimer(new QTimer(this))
    , timer1(new QTimer(this))
    , timer2(new QTimer(this))
    , timer3(new QTimer(this))
{
    serialPort = new QSerialPort(this);
    connect(server, &QTcpServer::newConnection, this, &TcpServer::onNewConnection);
    connect(sendTimer, &QTimer::timeout, this, &TcpServer::sendDataTimerTriggered);
    QObject::connect(serialPort, &QSerialPort::readyRead, this, [=](void) {
        qDebug() << "conectteeeeeeeee";
    });
}

void TcpServer::startServer(int port)
{
    if (!server->isListening() && server->listen(QHostAddress::Any, port)) {
        qDebug() << "Server started on port:" << port;
    } else {
        qDebug() << "Failed to start server:" << server->errorString();
    }
}

void TcpServer::stopServer()
{
    if (server->isListening()) {
        server->close();
        sendTimer->stop(); // Stop timer when server stops
        qDebug() << "Server stopped.";
    }
}

void TcpServer::sendData(const QString &data)
{
    QByteArray byteArray = data.toUtf8();
    for (QTcpSocket *client : qAsConst(clients)) {
        if (client->state() == QTcpSocket::ConnectedState) {
            client->write(byteArray);
            client->flush();
        }
    }
    qDebug() << "Sent data to clients:" << data;
}

void TcpServer::startInfiniteSend(const QString &data, int interval)
{
    sendDataString = data + "\r";
    sendTimer->setInterval(interval);
    sendTimer->start();
    qDebug() << "Started infinite send with data:" << data << " every " << interval << " ms.";
}

void TcpServer::stopInfiniteSend()
{
    sendTimer->stop();
    qDebug() << "Stopped infinite send.";
}

void TcpServer::sendRespDonePrinter()
{
    sendData("\x02"
             "030,0,0,1757,000,0,0,0,000,0,0,0\x03\r\n\x02"
             "001,0,0,0,1,2,6,0,00000000,1,000\x03\r\n\x02"
             "0000,0\x03\r\n");
}

void TcpServer::sendRespWeightTest()
{
    sendData("\x02"
             "030,0,0,1757,000,0,0,0,000,0,0,0\x03\r\n\x02"
             "001,0,0,0,1,2,6,0,00000000,1,000\x03\r\n\x02"
             "0000,0\x03\r\n");
}

void TcpServer::applySerialParams(const QString &portName, int idx)
{
    if (idx < 0 || idx >= 3)
        return;

    m_scales232[idx] = new ScaleInterface232(portName, this);
    m_scales232[idx]->applySerialParams(QSerialPort::Baud9600,
                                        QSerialPort::Data7,
                                        QSerialPort::EvenParity,
                                        QSerialPort::OneStop);
    hookSignals(idx);
    const bool ok = m_scales232[idx]->connect();
    if (ok) {
        //read continuos if need
        qDebug() << "===========connected " << idx;
    }
}

void TcpServer::sendFrameFake1(int ms, const QString &w)
{
    frame = QString("ST,+%1 g\r\n").arg(w).toUtf8();
    if (ms > 0)
        timer1->setInterval(ms);
    timer1->start();
    m_scales232[0]->sendData(frame);
}

void TcpServer::stopFake1()
{
    if (timer1->isActive()) {
        sendZeroData(0);
        timer1->stop();
    }
    // delete timer1;
}

void TcpServer::sendFrameFake2(int ms, const QString &w)
{
    frame1 = QString("020202+000.220ST,GS,+%1 kg\r\n").arg(w).toUtf8();
    if (ms > 0)
        timer2->setInterval(ms);
    timer2->start();
    m_scales232[1]->sendData(frame1);
}

void TcpServer::stopFake2()
{
    if (timer2->isActive()) {
        sendZeroData(1);
        timer2->stop();
    }
    // delete timer2;
}

void TcpServer::sendFrameFake3(int ms, const QString &w)
{
    frame2 = QString("030303+000.330ST,GS,+%1 kg\r\n").arg(w).toUtf8();
    if (ms > 0)
        timer3->setInterval(ms);
    timer3->start();
    m_scales232[2]->sendData(frame2);
}

void TcpServer::stopFake3()
{
    if (timer3->isActive()) {
        sendZeroData(2);
        timer3->stop();
    }
}

void TcpServer::sendZeroData(int u)
{
    if (u < 0 || u >= 3)
        return;

    QByteArray zeroFrame;
    if (u == 0) {
        zeroFrame = "ST,+0 g\r\n";
    } else {
        zeroFrame = "020202+000.220ST,GS,+0 kg\r\n";
    }

    if (m_scales232[u])
        m_scales232[u]->sendData(zeroFrame);

    qDebug() << "[FakeScale" << (u + 1) << "] Sent:" << zeroFrame;
}

void TcpServer::hookSignals(int idx)
{
    auto s = m_scales232[idx];
    if (!s)
        return;

    // Connection Stt
    QObject::connect(s, &ScaleInterface232::connectionStatusChanged, this, [this, idx](bool c) {

    });

    // Data weighing
    QObject::connect(s,
                     &ScaleInterface232::weightReceived,
                     this,
                     [this, idx](double w, const QString &u) {

                     });

    switch (idx) {
    case 0:
        timer1->setInterval(400);
        connect(timer1, &QTimer::timeout, this, [this]() {
            if (m_scales232[0])
                m_scales232[0]->sendData(frame);
            qDebug() << "[FakeScale1] Sent:" << frame;
        }, Qt::UniqueConnection);
        break;
    case 1:
        timer2->setInterval(400);
        connect(timer2, &QTimer::timeout, this, [this]() {
            if (m_scales232[1])
                m_scales232[1]->sendData(frame1);
            qDebug() << "[FakeScale2] Sent:" << frame1;
        }, Qt::UniqueConnection);
        break;
    case 2:
        timer3->setInterval(400);
        connect(timer3, &QTimer::timeout, this, [this]() {
            if (m_scales232[2])
                m_scales232[2]->sendData(frame2);
            qDebug() << "[FakeScale3] Sent:" << frame2;
        }, Qt::UniqueConnection);
        break;
    default:
        break;
    }
}

void TcpServer::sendDataTimerTriggered()
{
    sendData(sendDataString);
}

void TcpServer::onNewConnection()
{
    QTcpSocket *client = server->nextPendingConnection();
    clients.append(client);

    connect(client, &QTcpSocket::readyRead, this, &TcpServer::onReadyRead);
    connect(client, &QTcpSocket::disconnected, this, &TcpServer::onClientDisconnected);

    emit clientConnected(client->peerAddress().toString(), client->peerPort());
    qDebug() << "New client connected.";
}

void TcpServer::onReadyRead()
{
    QTcpSocket *client = qobject_cast<QTcpSocket *>(sender());
    if (client) {
        QString data = QString::fromUtf8(client->readAll());
        qDebug() << data;
        emit dataReceived(data);
    }
}

void TcpServer::onClientDisconnected()
{
    QTcpSocket *client = qobject_cast<QTcpSocket *>(sender());
    if (client) {
        clients.removeOne(client);
        client->deleteLater();
        emit clientDisconnected();
    }
}
