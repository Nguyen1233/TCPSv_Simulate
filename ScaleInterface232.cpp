#include "ScaleInterface232.h"
#include <QDebug>
#include <QRegularExpression>
#include <QTimer>

ScaleInterface232::ScaleInterface232(const QString &portName, QObject *parent)
    : QObject(parent)
    , portName(portName)
    , connected(false)
    , dataBuffer("")
{
    qDebug() << "Initializing ScaleInterface232 with port:" << portName;
    serialPort = new QSerialPort(this);
    serialPort->setPortName(portName);
    // Serial settings for A&D GF-1200 (per manual)
    serialPort->setBaudRate(QSerialPort::Baud9600);
    serialPort->setDataBits(QSerialPort::Data7);
    serialPort->setParity(QSerialPort::EvenParity);
    serialPort->setStopBits(QSerialPort::OneStop);
    serialPort->setFlowControl(QSerialPort::NoFlowControl);

    QObject::connect(serialPort, &QSerialPort::readyRead, this, &ScaleInterface232::handleReadyRead);
    QObject::connect(serialPort, &QSerialPort::errorOccurred, this, &ScaleInterface232::handleError);
}

ScaleInterface232::~ScaleInterface232()
{
    disconnect();
}

void ScaleInterface232::initialize()
{
    connect(); // Attempt to connect on initialization
}

bool ScaleInterface232::connect()
{
    if (serialPort->open(QIODevice::ReadWrite)) {
        connected = true;
        emit connectionStatusChanged(true);
        qDebug() << "Connected to" << portName;
        // Send cancel command to clear any ongoing requests
        cancelRequest();
        return true;
    } else {
        handleScaleError(ScaleError::PortNotConnected, serialPort->errorString());
        return false;
    }
}

void ScaleInterface232::disconnect()
{
    if (serialPort->isOpen())
        serialPort->close();
    markOpenClosed(false);
}

bool ScaleInterface232::isConnected() const
{
    return connected;
}

void ScaleInterface232::applySerialParams(int baudRate,
                                          QSerialPort::DataBits dataBits,
                                          QSerialPort::Parity parity,
                                          QSerialPort::StopBits stopBits)
{
    if (serialPort->isOpen())
        serialPort->close();

    serialPort->setBaudRate(baudRate);
    serialPort->setDataBits(dataBits);
    serialPort->setParity(parity);
    serialPort->setStopBits(stopBits);
    serialPort->setFlowControl(QSerialPort::NoFlowControl);
}

bool ScaleInterface232::sendCommand(ScaleCommand command)
{
    if (!connected) {
        handleScaleError(ScaleError::PortNotConnected, "Port not connected");
        emitStatusIfChanged(serialStatus());
        return false;
    }
    QString commandStr = scaleCommandToString(command);
    QByteArray data = commandStr.toLocal8Bit();
    qint64 bytesWritten = serialPort->write(data);
    if (bytesWritten == -1) {
        handleScaleError(ScaleError::WriteError, serialPort->errorString());
        emitStatusIfChanged(serialStatus());
        return false;
    }
    serialPort->flush();
    qDebug() << "Sent command:" << commandStr.trimmed();
    emitStatusIfChanged(serialStatus());
    return true;
}

bool ScaleInterface232::requestStableData()
{
    // Send cancel command first to ensure clean state
    cancelRequest();
    return sendCommand(ScaleCommand::Stable); // Send 'S' command
}

bool ScaleInterface232::requestStableDataEscP()
{
    cancelRequest();
    return sendCommand(ScaleCommand::StableEscP); // Send '<ESC>P' command
}

bool ScaleInterface232::cancelRequest()
{
    return sendCommand(ScaleCommand::Cancel); // Send 'C' command
}

bool ScaleInterface232::requestImmediateData()
{
    return sendCommand(ScaleCommand::ImmediateQ); // Send 'Q' command
}

bool ScaleInterface232::requestContinuousData()
{
    return sendCommand(ScaleCommand::Continuous); // Send 'SIR' command
}

void ScaleInterface232::handleReadyRead()
{
    const QByteArray data = serialPort->readAll();
    const QString dataStr = QString::fromLocal8Bit(data);
    dataBuffer += dataStr;

    qDebug() << "Received raw data:" << dataStr << "| Buffer:" << dataBuffer;

    int idx;
    while ((idx = dataBuffer.indexOf("\r\n")) >= 0) {
        const QString msg = dataBuffer.left(idx);
        dataBuffer.remove(0, idx + 2);
        const QString trimmedMsg = msg.trimmed();
        if (trimmedMsg.isEmpty())
            continue;
        //for who need ra value scale
        emit dataReceived(trimmedMsg);

        double weightKg = 0.0;
        QString unitRaw;
        if (parseWeightData(trimmedMsg, weightKg, unitRaw)) {
            emit weightReceived(weightKg, unitRaw);
            // emit weightReceived(weightKg, unitRaw);
            qDebug() << "[OK] Weight =" << weightKg << unitRaw << " from" << trimmedMsg;
        } else {
            handleScaleError(ScaleError::InvalidDataFormat, "Invalid data format: " + trimmedMsg);
            qWarning() << "[WARN] Parse failed for:" << trimmedMsg;
        }
    }
    emitStatusIfChanged(serialStatus());
}

bool ScaleInterface232::parseWeightData(const QString &data, double &weight, QString &unit)
{
    static const QRegularExpression re(R"(([+\-]?\d+(?:\.\d+)?)[ ]*(kg|g))",
                                       QRegularExpression::CaseInsensitiveOption);

    QRegularExpressionMatchIterator it = re.globalMatch(data);
    QRegularExpressionMatch lastMatch;
    while (it.hasNext())
        lastMatch = it.next();

    if (!lastMatch.hasMatch())
        return false;

    bool ok = false;
    const double v = lastMatch.captured(1).toDouble(&ok);
    if (!ok)
        return false;

    unit = lastMatch.captured(2).toLower();

    // if (unit == "kg") {
    //     weight = v;
    // } else if (unit == "g") {
    //     weight = v;
    // } else {
    //     weight = v;
    // }
    weight = v;
    return true;
}

void ScaleInterface232::handleScaleError(ScaleError error, const QString &errorString)
{
    QString errorMessage;
    switch (error) {
    case ScaleError::NoError:
        errorMessage = "No error occurred";
        break;
    case ScaleError::PortNotConnected:
        errorMessage = "Serial port is not connected: " + errorString;
        break;
    case ScaleError::WriteError:
        errorMessage = "Failed to write to serial port: " + errorString;
        break;
    case ScaleError::InvalidDataFormat:
        errorMessage = "Invalid data format received: " + errorString;
        break;
    case ScaleError::UnstableData:
        errorMessage = "Received unstable data: " + errorString;
        break;
    case ScaleError::WeightConversionError:
        errorMessage = "Failed to convert weight: " + errorString;
        break;
    case ScaleError::UnitNotFound:
        errorMessage = "Unit not found in data: " + errorString;
        break;
    default:
        errorMessage = "Unknown error: " + errorString;
        break;
    }

    qDebug() << "Scale error:" << errorMessage;
    emit errorOccurred(error, errorMessage);
}

void ScaleInterface232::emitStatusIfChanged(const QString &newText)
{
    if (m_statusText == newText)
        return;
    m_statusText = newText;
    emit statusChanged();
}

void ScaleInterface232::markOpenClosed(bool open)
{
    if (connected == open) {
        emitStatusIfChanged(serialStatus());
        return;
    }
    connected = open;
    emit connectionStatusChanged(open);
    emitStatusIfChanged(serialStatus());
}

void ScaleInterface232::handleError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::NoError)
        return;
    handleScaleError(ScaleError::PortNotConnected, serialPort->errorString());

    if (serialPort->isOpen())
        serialPort->close();

    markOpenClosed(false);
    emitStatusIfChanged(serialStatus());
}

QString ScaleInterface232::serialStatus() const
{
    if (!serialPort)
        return "No Port";

    QString s = serialPort->isOpen() ? QString("Open (%1 @ %2, %3,%4,%5)")
                                           .arg(serialPort->portName())
                                           .arg(serialPort->baudRate())
                                           .arg((int) serialPort->dataBits())
                                           .arg((int) serialPort->parity())
                                           .arg((int) serialPort->stopBits())
                                     : QString("Closed (%1)").arg(serialPort->portName());

    if (serialPort->error() != QSerialPort::NoError) {
        s += QString(" | Error: %1").arg(serialPort->errorString());
    }
    return s;
}

void ScaleInterface232::sendData(const QString &frame)
{
    if (!serialPort)
        return;
    QByteArray data = frame.toLocal8Bit();
    serialPort->write(data);
    serialPort->flush();
}
