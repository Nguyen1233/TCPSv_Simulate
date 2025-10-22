#ifndef SCALEINTERFACE232_H
#define SCALEINTERFACE232_H

#include <QObject>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include "ScaleCommand.h"

class ScaleInterface232 : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString statusText READ serialStatus NOTIFY statusChanged)
public:
    explicit ScaleInterface232(const QString &portName, QObject *parent = nullptr);
    ~ScaleInterface232();

    void initialize();
    Q_SLOT void initializeSlot() { initialize(); }
    bool connect();
    void disconnect();
    bool isConnected() const;
    void applySerialParams(int baudRate,
                           QSerialPort::DataBits dataBits,
                           QSerialPort::Parity parity,
                           QSerialPort::StopBits stopBits);

    // Commands for A&D GF-1200 scale
    bool requestStableData();
    bool requestStableDataEscP();
    bool cancelRequest();
    bool requestImmediateData();
    bool requestContinuousData();

    QString serialStatus() const;
    Q_SLOT void sendData(const QString &frame);

signals:
    void weightReceived(double weight, const QString &unit);
    void dataReceived(const QString &data);
    void errorOccurred(ScaleError error, const QString &errorString);
    void connectionStatusChanged(bool connected);

    void statusChanged();

private slots:
    void handleReadyRead();
    void handleError(QSerialPort::SerialPortError error);

private:
    bool sendCommand(ScaleCommand command);
    bool parseWeightData(const QString &data, double &weight, QString &unit);
    void handleScaleError(ScaleError error, const QString &errorString);
    void emitStatusIfChanged(const QString &newText);
    void markOpenClosed(bool open);

    QSerialPort *serialPort;
    QString portName;
    bool connected;
    QString dataBuffer;
    QString m_statusText;
};

#endif // SCALEINTERFACE232_H
