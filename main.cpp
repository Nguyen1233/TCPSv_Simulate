#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "TcpServer.h"

#include <QString>
#include <QRegularExpression>

QString incrementCode(const QString &code)
{
    QString trimmed = code.trimmed();
    QRegularExpression re("^(.*)-(\\d+)$");
    QRegularExpressionMatch match = re.match(trimmed);

    if (match.hasMatch()) {
        QString prefix = match.captured(1);     // phần trước dấu '-'
        QString numStr = match.captured(2);      // phần số, ví dụ "009"
        int num = numStr.toInt() + 1;

        // Giữ độ dài cũ bằng cách padding bằng '0'
        QString padded = QString("%1").arg(num, numStr.length(), 10, QLatin1Char('0'));

        return /*prefix + "-" +*/ padded;
    }

    // Nếu không khớp pattern, trả lại nguyên chuỗi
    return trimmed;
}

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    TcpServer tcpServer;

    engine.rootContext()->setContextProperty("tcpServer", &tcpServer);

    QString m_code = "INK251021-009 ";
    QString newCode = incrementCode(m_code);
    qDebug() << "Before:" << m_code << "→ After:" << newCode;

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
