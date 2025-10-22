#include "TcpServer.h"
#include <QDebug>
#include <QThread>

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
    // Send to each client via its worker (if available). This ensures writes happen in the
    // dedicated client thread to avoid cross-thread socket access and contention.
    for (int i = 0; i < clientWorkers.size(); ++i) {
        ClientWorker *w = clientWorkers.value(i, nullptr);
        if (w) {
            QMetaObject::invokeMethod(w, "sendData", Qt::QueuedConnection, Q_ARG(QByteArray, byteArray));
        } else {
            // fallback to raw socket if no worker
            QTcpSocket *client = clients.value(i, nullptr);
            if (client && client->state() == QTcpSocket::ConnectedState) {
                client->write(byteArray);
                client->flush();
            }
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
    // Create scale and its dedicated thread so serial I/O runs separately
    ScaleInterface232 *scale = new ScaleInterface232(portName);
    QThread *t = new QThread(this);
    scale->moveToThread(t);

    // Apply params on the object's thread using queued invocation
    QMetaObject::invokeMethod(scale, [scale]() {
        scale->applySerialParams(QSerialPort::Baud9600,
                                 QSerialPort::Data7,
                                 QSerialPort::EvenParity,
                                 QSerialPort::OneStop);
    }, Qt::QueuedConnection);

    // Hook signals (note: signals are thread-safe across threads)
    hookSignals(idx);

    // Store pointer and start thread
    m_scales232[idx] = scale;
    t->start();

    // Initialize (attempt open) inside the scale thread
    QMetaObject::invokeMethod(scale, "initializeSlot", Qt::QueuedConnection);

    qDebug() << "Scale thread started for idx" << idx << "port" << portName;
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
        if (m_scales232[0])
            QMetaObject::invokeMethod(m_scales232[0], "sendData", Qt::QueuedConnection, Q_ARG(QString, QString::fromUtf8(frame)));
    } else {
        frame = "020202+000.220ST,GS,+0 kg\r\n";
        if (m_scales232[1])
            QMetaObject::invokeMethod(m_scales232[1], "sendData", Qt::QueuedConnection, Q_ARG(QString, QString::fromUtf8(frame)));
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
    // Forward raw scale data to connected TCP clients (append CRLF to restore framing)
    QObject::connect(s, &ScaleInterface232::dataReceived, this, [this](const QString &raw) {
        const QString framed = raw + "\r\n";
        // sendData will route the bytes to each client via their worker threads
        sendData(framed);
        qDebug() << "Forwarded scale data to clients:" << framed;
    }, Qt::QueuedConnection);
    timer1->setInterval(400);
    connect(timer1, &QTimer::timeout, this, [this]() {
        // frame = "ST,+2000 g\r\n"; // chuỗi mô phỏng cân
        // m_scales232[]->write(frame);
        if (m_scales232[0])
            QMetaObject::invokeMethod(m_scales232[0], "sendData", Qt::QueuedConnection, Q_ARG(QString, QString::fromUtf8(frame)));
        // serialPort->flush();
        qDebug() << "[FakeScale1] Sent:" << frame;
    });

    timer2->setInterval(400);
    connect(timer2, &QTimer::timeout, this, [this]() {
        // frame1 = "020202+000.220ST,GS,+5 kg\r\n"; // chuỗi mô phỏng cân
        // serialPort->write(frame);
        // serialPort->flush();
        if (m_scales232[1])
            QMetaObject::invokeMethod(m_scales232[1], "sendData", Qt::QueuedConnection, Q_ARG(QString, QString::fromUtf8(frame1)));
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
    if (!client)
        return;

    // Create worker and thread for this client
    QThread *thread = new QThread(this);
    ClientWorker *worker = new ClientWorker(client);

    // Move socket to the new thread so its I/O runs there
    client->moveToThread(thread);
    worker->moveToThread(thread);

    // When thread finishes, delete worker
    connect(thread, &QThread::finished, worker, &QObject::deleteLater);

    // Connect socket signals to worker slots (queued) so they run in worker thread
    connect(client, &QTcpSocket::readyRead, worker, &ClientWorker::onReadyRead, Qt::QueuedConnection);
    connect(client, &QTcpSocket::disconnected, worker, &ClientWorker::onDisconnected, Qt::QueuedConnection);

    // Hook worker signals back to server (queued) and forward actual data
    connect(worker, &ClientWorker::dataReceived, this, [this](const QString &d) {
        emit dataReceived(d);
        qDebug() << "Forwarded data from worker:" << d;
    }, Qt::QueuedConnection);

    connect(worker, &ClientWorker::disconnected, this, &TcpServer::onClientDisconnected, Qt::QueuedConnection);
    connect(worker, &ClientWorker::disconnected, thread, &QThread::quit);
    connect(worker, &ClientWorker::finished, thread, &QThread::quit);

    // Start the thread
    thread->start();

    // Keep bookkeeping
    clients.append(client);
    clientThreads.append(thread);
    clientWorkers.append(worker);

    emit clientConnected(client->peerAddress().toString(), client->peerPort());
    qDebug() << "New client connected and worker thread started.";
}

void TcpServer::onReadyRead()
{
    // This slot can be invoked either directly from a QTcpSocket signal or from
    // a ClientWorker::dataReceived signal. Handle both cases.
    QTcpSocket *client = qobject_cast<QTcpSocket *>(sender());
    if (client) {
        QString data = QString::fromUtf8(client->readAll());
        qDebug() << data;
        emit dataReceived(data);
        return;
    }

    // If called via worker signal, sender is the ClientWorker and the
    // first argument is the data; Qt queued connection will package it so
    // we can retrieve it via QMetaObject invocation context. Simpler: cast
    // sender to ClientWorker and rely on its signal to carry the data.
    ClientWorker *worker = qobject_cast<ClientWorker *>(sender());
    if (worker) {
        // Worker already emitted dataReceived(QString) which was connected here,
        // but because we connected it with this slot signature we receive a call
        // with no explicit parameter. Instead we rely on the signal payload via
        // QObject::sender() is not giving us the parameter. To keep behavior,
        // change the connect in onNewConnection to bind directly to a lambda
        // — but to keep patch minimal, we will not duplicate logic here.
        // As a fallback, just emit that data arrived (worker already emitted it
        // so the server's higher-level listeners will still receive it). Log only.
        qDebug() << "Data received from worker";
    }
}

void TcpServer::onClientDisconnected()
{
    // Try to handle both direct socket sender and worker sender
    QTcpSocket *client = qobject_cast<QTcpSocket *>(sender());
    if (client) {
        int idx = clients.indexOf(client);
        if (idx >= 0) {
            clients.removeAt(idx);
            // stop and cleanup thread/worker
            ClientWorker *w = clientWorkers.value(idx, nullptr);
            QThread *t = clientThreads.value(idx, nullptr);
            if (w) {
                QMetaObject::invokeMethod(w, "shutdown", Qt::QueuedConnection);
            }
            if (t) {
                t->quit();
                t->wait(100);
                t->deleteLater();
            }
            clientWorkers.removeAt(idx);
            clientThreads.removeAt(idx);
        }
        client->deleteLater();
        emit clientDisconnected();
        return;
    }

    ClientWorker *worker = qobject_cast<ClientWorker *>(sender());
    if (worker) {
        int idx = clientWorkers.indexOf(worker);
        if (idx >= 0) {
            QThread *t = clientThreads.value(idx, nullptr);
            QTcpSocket *s = clients.value(idx, nullptr);
            clients.removeAt(idx);
            clientWorkers.removeAt(idx);
            clientThreads.removeAt(idx);
            if (s)
                s->deleteLater();
            if (t) {
                t->quit();
                t->wait(100);
                t->deleteLater();
            }
        }
        emit clientDisconnected();
    }
}
