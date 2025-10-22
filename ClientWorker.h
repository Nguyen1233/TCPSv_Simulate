#ifndef CLIENTWORKER_H
#define CLIENTWORKER_H

#include <QObject>
#include <QTcpSocket>

class ClientWorker : public QObject
{
    Q_OBJECT
public:
    explicit ClientWorker(QTcpSocket *socket, QObject *parent = nullptr);
    ~ClientWorker();

    QTcpSocket *socket() const { return m_socket; }

public slots:
    void sendData(const QByteArray &data);
    void shutdown();

    // These are intentionally public so external code can connect socket signals to them
    void onReadyRead();
    void onDisconnected();

signals:
    void finished();
    void disconnected();
    void dataReceived(const QString &data);


private:
    QTcpSocket *m_socket;
};

#endif // CLIENTWORKER_H
