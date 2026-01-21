import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15
import QtQuick.Layouts 1.15
import Qt5Compat.GraphicalEffects

ApplicationWindow {
    visible: true
    width: 900
    height: 700
    title: "TCP Server"

    // Material style
    Material.theme: Material.Light
    Material.accent: Material.Blue

    Component.onCompleted: tcpServer.startServer(8888)

    Rectangle {
        anchors.fill: parent
        color: "#f0f0f0"

        ColumnLayout {
            anchors.fill: parent
            spacing: 20

            GroupBox {
                id: connectBox
                title: "Simulate Device"
                Layout.fillWidth: true
                padding: 20

                ColumnLayout {
                    spacing: 15

                    RowLayout {
                        id: portRow
                        Layout.fillWidth: true
                        spacing: 10

                        TextField {
                            id: portField
                            placeholderText: "Enter port (e.g., 9004)"
                            font.pixelSize: 16
                            Layout.fillWidth: true
                        }

                        Rectangle {
                            id: imageContainer
                            width: 90
                            height: portField.implicitHeight
                            radius: 8
                            border.color: "#cccccc"
                            clip: true
                            Layout.alignment: Qt.AlignVCenter

                            Image {
                                id: dogImage
                                anchors.fill: parent
                                smooth: true
                                visible: true
                                fillMode: Image.PreserveAspectCrop
                                source: "https://placedog.net/500/250?id=1"
                            }

                            OpacityMask {
                                anchors.fill: dogImage
                                source: dogImage
                                maskSource: Rectangle {
                                    width: dogImage.width
                                    height: dogImage.height
                                    radius: Math.max(0, imageContainer.radius - imageContainer.border.width)
                                    color: "white"
                                }
                            }

                            Timer {
                                id: imageTimer
                                interval: 25 * 1000 // 25s
                                running: true
                                repeat: true
                                property int counter: 1
                                onTriggered: {
                                    counter++
                                    dogImage.source = "https://placedog.net/500/250?id=" + counter
                                    if (counter === 249) counter = 1
                                }
                            }
                        }
                    }

                    RowLayout {
                        spacing: 10
                        Layout.alignment: Qt.AlignHCenter

                        Button {
                            text: "Start"
                            Layout.preferredWidth: 160
                            Layout.preferredHeight: 40
                            font.pixelSize: 14
                            onClicked: {
                                const port = parseInt(portField.text)
                                if (!isNaN(port)) tcpServer.startServer(port)
                                else console.log("Invalid port number.")
                            }
                        }

                        Button {
                            text: "Stop"
                            Layout.preferredWidth: 160
                            Layout.preferredHeight: 40
                            font.pixelSize: 14
                            onClicked: tcpServer.stopServer()
                        }

                        Button {
                            text: "TestWeighing"
                            Layout.preferredWidth: 160
                            Layout.preferredHeight: 40
                            font.pixelSize: 14
                            onClicked: tcpServer.sendData(portField.text)
                        }

                        Button {
                            text: "SendResponse"
                            Layout.preferredWidth: 160
                            Layout.preferredHeight: 40
                            font.pixelSize: 14
                            onClicked: {
                                tcpServer.sendRespDonePrinter()
                                tcpServer.sendData(0)
                            }
                        }
                    }
                }
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing: 10

                Frame {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    ScrollView {
                        anchors.fill: parent
                        clip: true

                        TextArea {
                            id: logArea
                            anchors.fill: parent
                            readOnly: true
                            wrapMode: TextArea.Wrap
                            background: Rectangle {
                                color: "#ffffff"
                                border.color: "#cccccc"
                                border.width: 1
                            }
                        }
                    }
                }

                Frame {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    ColumnLayout {
                        anchors.fill: parent
                        spacing: 8
                        RowLayout {
                            // anchors.fill: parent
                            Button {
                                id: connectBt1
                                text: "Connect"
                                onClicked: {
                                    if(connectBt1.text==="Connect") {
                                        connectBt1.text = "Disconnect"
                                        tcpServer.applySerialParams(precision.text,0)
                                    }
                                    else {
                                        connectBt1.text = "Connect"
                                    }
                                }
                            }

                            TextField {
                                id: precision
                                text: "COM8"
                            }

                            TextField {
                                id: testWeigh1
                                placeholderText: "testWeigh1"
                            }

                            TextField {
                                id: timer1intv
                                placeholderText: "IntervalW1"
                            }

                            Button {
                                id: continuesData1
                                text: "StartContinue"
                                onClicked: {
                                    if(continuesData1.text==="StartContinue") {
                                        continuesData1.text = "StopContinue"
                                        tcpServer.sendFrameFake1(timer1intv.text,testWeigh1.text)
                                    }
                                    else {
                                        continuesData1.text = "StartContinue"
                                        tcpServer.stopFake1()
                                    }
                                }
                            }
                        }

                        RowLayout {
                            // anchors.fill: parent
                            Button {
                                id: connectBt2
                                text: "Connect"
                                onClicked: {
                                    if(connectBt2.text==="Connect") {
                                        connectBt2.text = "Disconnect"
                                        tcpServer.applySerialParams(industrial.text,1)
                                    }
                                    else {
                                        connectBt2.text = "Connect"
                                    }
                                }
                            }

                            TextField {
                                id: industrial
                                text: "COM9"
                            }

                            TextField {
                                id: testWeigh2
                                placeholderText: "testWeigh2"
                            }

                            TextField {
                                id: timer2intv
                                placeholderText: "IntervalW2"
                            }


                            Button {
                                id: continuesData2
                                text: "StartContinue"
                                onClicked: {
                                    if(continuesData2.text==="StartContinue") {
                                        continuesData2.text = "StopContinue"
                                        tcpServer.sendFrameFake2(timer2intv.text,testWeigh2.text)
                                    }
                                    else {
                                        continuesData2.text = "StartContinue"
                                        tcpServer.stopFake2()
                                    }
                                }
                            }
                        }
                        RowLayout {
                            Button {
                                id: connectBt3
                                text: "Connect"
                                onClicked: {
                                    if(connectBt3.text==="Connect") {
                                        connectBt3.text = "Disconnect"
                                        tcpServer.applySerialParams(thirdScalePort.text,2)
                                    }
                                    else {
                                        connectBt3.text = "Connect"
                                    }
                                }
                            }

                            TextField {
                                id: thirdScalePort
                                text: "COM10"
                            }

                            TextField {
                                id: testWeigh3
                                placeholderText: "testWeigh3"
                            }

                            TextField {
                                id: timer3intv
                                placeholderText: "IntervalW3"
                            }

                            Button {
                                id: continuesData3
                                text: "StartContinue"
                                onClicked: {
                                    if(continuesData3.text==="StartContinue") {
                                        continuesData3.text = "StopContinue"
                                        tcpServer.sendFrameFake3(timer3intv.text,testWeigh3.text)
                                    }
                                    else {
                                        continuesData3.text = "StartContinue"
                                        tcpServer.stopFake3()
                                    }
                                }
                            }
                        }
                        Item { Layout.fillHeight: true }
                    }
                }
            }
        }

        Connections {
            target: tcpServer

            function onClientConnected(address, port) {
                logArea.text += "Client connected: " + address + ":" + port + "\n"
                logArea.cursorPosition = logArea.text.length
            }
            function onClientDisconnected() {
                logArea.text += "Client disconnected\n"
                logArea.cursorPosition = logArea.text.length
            }
            function onDataReceived(data) {
                logArea.text += "Received: " + data + "\n"
                logArea.cursorPosition = logArea.text.length
            }
        }
    }
}
