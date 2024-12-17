#include "tcpserver.h"
#include <QDebug>

TcpServer::TcpServer(QObject *parent)
    : QObject(parent),
    server(new QTcpServer(this)),
    sendTimer(new QTimer(this)) {

    connect(server, &QTcpServer::newConnection, this, &TcpServer::onNewConnection);
    connect(sendTimer, &QTimer::timeout, this, &TcpServer::sendDataTimerTriggered);
}

void TcpServer::startServer(int port) {
    if (!server->isListening() && server->listen(QHostAddress::Any, port)) {
        qDebug() << "Server started on port:" << port;
    } else {
        qDebug() << "Failed to start server:" << server->errorString();
    }
}

void TcpServer::stopServer() {
    if (server->isListening()) {
        server->close();
        sendTimer->stop(); // Stop timer when server stops
        qDebug() << "Server stopped.";
    }
}

void TcpServer::sendData(const QString &data) {
    QByteArray byteArray = data.toUtf8();
    for (QTcpSocket *client : qAsConst(clients)) {
        if (client->state() == QTcpSocket::ConnectedState) {
            client->write(byteArray);
            client->flush();
        }
    }
    qDebug() << "Sent data to clients:" << data;
}

void TcpServer::startInfiniteSend(const QString &data, int interval) {
    sendDataString = data + "\r";
    sendTimer->setInterval(interval);
    sendTimer->start();
    qDebug() << "Started infinite send with data:" << data << " every " << interval << " ms.";
}

void TcpServer::stopInfiniteSend() {
    sendTimer->stop();
    qDebug() << "Stopped infinite send.";
}

void TcpServer::sendDataTimerTriggered() {
    sendData(sendDataString);
}

void TcpServer::onNewConnection() {
    QTcpSocket *client = server->nextPendingConnection();
    clients.append(client);

    connect(client, &QTcpSocket::readyRead, this, &TcpServer::onReadyRead);
    connect(client, &QTcpSocket::disconnected, this, &TcpServer::onClientDisconnected);

    emit clientConnected(client->peerAddress().toString(), client->peerPort());
    qDebug() << "New client connected.";
}

void TcpServer::onReadyRead() {
    QTcpSocket *client = qobject_cast<QTcpSocket *>(sender());
    if (client) {
        QString data = QString::fromUtf8(client->readAll());
        emit dataReceived(data);
    }
}

void TcpServer::onClientDisconnected() {
    QTcpSocket *client = qobject_cast<QTcpSocket *>(sender());
    if (client) {
        clients.removeOne(client);
        client->deleteLater();
        emit clientDisconnected();
    }
}
