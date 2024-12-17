#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>

class TcpServer : public QObject {
    Q_OBJECT
public:
    explicit TcpServer(QObject *parent = nullptr);

    Q_INVOKABLE void startServer(int port);
    Q_INVOKABLE void stopServer();
    Q_INVOKABLE void sendData(const QString &data);
    Q_INVOKABLE void startInfiniteSend(const QString &data, int interval = 1000); // interval in milliseconds
    Q_INVOKABLE void stopInfiniteSend();

signals:
    void clientConnected(QString address, int port);
    void clientDisconnected();
    void dataReceived(QString data);

private:
    QTcpServer *server;
    QList<QTcpSocket *> clients;
    QTimer *sendTimer; // Timer for infinite sending
    QString sendDataString; // Data to send repeatedly

private slots:
    void onNewConnection();
    void onReadyRead();
    void onClientDisconnected();
    void sendDataTimerTriggered();
};

#endif // TCPSERVER_H
