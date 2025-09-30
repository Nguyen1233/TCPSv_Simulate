#include "TcpServer.h"
#include <QDebug>

TcpServer::TcpServer(QObject *parent)
    : QObject(parent)
    , server(new QTcpServer(this))
    , sendTimer(new QTimer(this))
    , timer1(new QTimer(this))
    , timer2(new QTimer(this))
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
    // timer1 = new QTimer(this);
    // timer1->setInterval(ms);
    frame = QString("ST,+%1 g\r\n").arg(w).toUtf8();
    timer1->start();
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
    // timer2 = new QTimer(this);
    // timer2->setInterval(ms);
    frame1 = QString("020202+000.220ST,GS,+%1 kg\r\n").arg(w).toUtf8();
    timer2->start();
}

void TcpServer::stopFake2()
{
    if (timer2->isActive()) {
        sendZeroData(8);
        timer2->stop();
    }
    // delete timer2;
}

void TcpServer::sendZeroData(int u)
{
    QByteArray frame;
    if (u == 0) {
        frame = "ST,+0 g\r\n";
        m_scales232[0]->sendData(frame);
    } else {
        frame = "020202+000.220ST,GS,+0 kg\r\n";
        m_scales232[1]->sendData(frame);
    }

    qDebug() << "[FakeScale1] Sent:" << frame;
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
    timer1->setInterval(400);
    connect(timer1, &QTimer::timeout, this, [this]() {
        // frame = "ST,+2000 g\r\n"; // chuỗi mô phỏng cân
        // m_scales232[]->write(frame);
        m_scales232[0]->sendData(frame);
        // serialPort->flush();
        qDebug() << "[FakeScale1] Sent:" << frame;
    });

    timer2->setInterval(400);
    connect(timer2, &QTimer::timeout, this, [this]() {
        // frame1 = "020202+000.220ST,GS,+5 kg\r\n"; // chuỗi mô phỏng cân
        // serialPort->write(frame);
        // serialPort->flush();
        m_scales232[1]->sendData(frame1);
        // serialPort->flush();
        qDebug() << "[FakeScale2] Sent:" << frame1;
    });
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
