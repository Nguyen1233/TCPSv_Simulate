import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15 // Import Material style

ApplicationWindow {
    visible: true
    width: 1080
    height: 877
    title: "TCP Server"

    // Set Material style for the application
    Material.theme: Material.Light
    Material.accent: Material.Blue

    Component.onCompleted: {
        tcpServer.startServer(9100);
    }

    Rectangle {
        anchors.fill: parent
        color: "#f0f0f0" // Light background for better contrast

        Column {
            id: mainColumn
            anchors.centerIn: parent
            spacing: 20
            width: parent.width  // Use 80% of window width for better scaling
            height: parent.height

            // Server control group
            GroupBox {
                title: "Server Controls"
                width: parent.width
                padding: 20

                Column {
                    spacing: 15
                    width: parent.width

                    TextField {
                        id: portField
                        placeholderText: "Enter port (e.g., 9004)"
                        width: parent.width
                        font.pixelSize: 16
                    }

                    Row {
                        spacing: 10
                        anchors.horizontalCenter: parent.horizontalCenter

                        Button {
                            text: "Start Server"
                            width: 150
                            height: 40
                            font.pixelSize: 14
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
                            width: 150
                            height: 40
                            font.pixelSize: 14
                            onClicked: tcpServer.stopServer()
                        }

                        Button {
                            text: "SendResponse"
                            width: 150
                            height: 40
                            font.pixelSize: 14
                            onClicked: tcpServer.sendRespDonePrinter()
                        }
                    }
                }
            }

            // Log area with ScrollView
            // GroupBox {
            //     title: "Server Log"
            //     width: parent.width
            //     padding: 20

                ScrollView {
                    id: logScrollView
                    width: parent.width
                    height: 800 // Fixed height for the scrollable area
                    clip: true // Ensure content outside the view is clipped

                    TextArea {
                        id: logArea
                        width: parent.width
                        readOnly: true
                        font.pixelSize: 14
                        wrapMode: TextArea.Wrap
                        background: Rectangle {
                            color: "#ffffff"
                            border.color: "#cccccc"
                            border.width: 1
                        }
                    }
                }
            // }
        }

        Connections {
            target: tcpServer

            function onClientConnected(address, port) {
                logArea.text += "Client connected: " + address + ":" + port + "\n";
                logArea.cursorPosition = logArea.text.length; // Auto-scroll to bottom
            }

            function onClientDisconnected() {
                logArea.text += "Client disconnected\n";
                logArea.cursorPosition = logArea.text.length;
            }

            function onDataReceived(data) {
                logArea.text += "Received: " + data + "\n";
                logArea.cursorPosition = logArea.text.length;
            }
        }
    }
}
