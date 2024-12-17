#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include "TcpServer.h"
#include <QQmlContext>

int main(int argc, char *argv[])
{
    // qputenv("QT_IM_MODULE", QByteArray("qtvirtualkeyboard"));

    QGuiApplication app(argc, argv);
    // NetSndRcv m_NetSndRcv;


    QQmlApplicationEngine engine;
    TcpServer tcpServer;

    engine.rootContext()->setContextProperty("tcpServer", &tcpServer);
    const QUrl url(QStringLiteral("qrc:/TCPsv/main.qml"));
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreated,
        &app,
        [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
        },
        Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
