import QtQuick 2.15
import QtQuick.Controls 2.15

ApplicationWindow {
    visible: true
    width: 640
    height: 480
    title: "TCP Server - Infinite Send"

    Column {
        spacing: 10
        anchors.centerIn: parent

        TextField {
            id: portField
            placeholderText: "Enter port (e.g., 9004)"
        }

        Button {
            text: "Start Server"
            onClicked: {
                const port = parseInt(portField.text);
                if (!isNaN(port)) {
                    tcpServer.startServer(port);
                } else {
                    console.log("Invalid port number.");
                }
            }
        }

        Button {
            text: "Stop Server"
            onClicked: tcpServer.stopServer()
        }

        TextField {
            id: dataField
            placeholderText: "Enter data to send (e.g., AM12345678)"
        }

        TextField {
            id: intervalField
            placeholderText: "Enter interval in ms (e.g., 1000)"
        }

        Button {
            text: "Start Infinite Send"
            onClicked: {
                const data = dataField.text;
                const interval = parseInt(intervalField.text);
                if (data.length > 0 && !isNaN(interval)) {
                    tcpServer.startInfiniteSend(data, interval);
                } else {
                    console.log("Invalid data or interval.");
                }
            }
        }

        Button {
            text: "Stop Infinite Send"
            onClicked: tcpServer.stopInfiniteSend()
        }

        TextArea {
            id: logArea
            width: parent.width * 0.8
            height: parent.height * 0.4
            readOnly: true
        }

        Connections {
            target: tcpServer

            function onClientConnected(address, port) {
                logArea.text += "Client connected: " + address + ":" + port + "\n";
            }

            function onClientDisconnected() {
                logArea.text += "Client disconnected\n";
            }

            function onDataReceived(data) {
                logArea.text += "Received: " + data + "\n";
            }
        }
    }
}
