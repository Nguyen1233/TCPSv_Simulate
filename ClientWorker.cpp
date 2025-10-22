#include "ClientWorker.h"
#include <QDebug>

ClientWorker::ClientWorker(QTcpSocket *socket, QObject *parent)
    : QObject(parent)
    , m_socket(socket)
{
    Q_UNUSED(socket)
}

ClientWorker::~ClientWorker()
{
    if (m_socket) {
        m_socket->deleteLater();
        m_socket = nullptr;
    }
}

void ClientWorker::sendData(const QByteArray &data)
{
    if (!m_socket)
        return;
    // write is safe to call from this thread since socket lives here
    m_socket->write(data);
    m_socket->flush();
}

void ClientWorker::shutdown()
{
    if (m_socket) {
        m_socket->disconnectFromHost();
    }
    emit finished();
}

void ClientWorker::onReadyRead()
{
    if (!m_socket)
        return;
    const QString data = QString::fromUtf8(m_socket->readAll());
    emit dataReceived(data);
}

void ClientWorker::onDisconnected()
{
    emit disconnected();
}
