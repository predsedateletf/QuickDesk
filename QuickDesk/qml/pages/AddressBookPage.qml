import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "../component"
import QuickDesk 1.0

Item {
    id: root

    required property var mainController

    signal showToast(string message, int toastType)
    signal connectRequested(string deviceId, string accessCode)

    AddressBookManager {
        id: book

        onConnectRequested: function(deviceId, accessCode) {
            root.connectRequested(deviceId, accessCode)
        }
    }

    Component.onCompleted: book.reload()

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 12

        RowLayout {
            Layout.fillWidth: true

            Text {
                text: qsTr("Адресная книга")
                font.pixelSize: 22
                font.bold: true
                color: Theme.text
            }

            Item { Layout.fillWidth: true }

            Button {
                text: qsTr("Обновить")
                enabled: book.loggedIn && !book.loading
                onClicked: book.reload()
            }
        }

        Text {
            Layout.fillWidth: true
            visible: !book.loggedIn
            text: qsTr("Войдите в аккаунт, чтобы видеть адресную книгу.")
            color: Theme.textSecondary
            font.pixelSize: 14
            wrapMode: Text.WordWrap
        }

        // книги
        Flow {
            Layout.fillWidth: true
            spacing: 8
            visible: book.loggedIn && book.books.length > 0

            Repeater {
                model: book.books

                Button {
                    text: modelData.name + " (" + modelData.contacts + ")"
                    checkable: true
                    checked: book.currentBook === modelData.id
                    onClicked: book.selectBook(modelData.id)
                }
            }
        }

        Text {
            Layout.fillWidth: true
            visible: book.loggedIn && book.books.length === 0 && !book.loading
            text: qsTr("Вам пока не открыт доступ ни к одной книге.")
            color: Theme.textSecondary
            font.pixelSize: 14
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "transparent"
            border.color: Theme.border
            border.width: 1
            radius: 8

            ListView {
                anchors.fill: parent
                anchors.margins: 8
                clip: true
                model: book.entries
                spacing: 4

                delegate: Rectangle {
                    width: ListView.view.width
                    height: 56
                    radius: 6
                    color: "transparent"

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 8
                        anchors.rightMargin: 8

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 2

                            Text {
                                text: modelData.name
                                color: Theme.text
                                font.pixelSize: 14
                                elide: Text.ElideRight
                                Layout.fillWidth: true
                            }

                            Text {
                                text: modelData.deviceId + (modelData.note ? " — " + modelData.note : "")
                                color: Theme.textSecondary
                                font.pixelSize: 12
                                elide: Text.ElideRight
                                Layout.fillWidth: true
                            }
                        }

                        Button {
                            text: qsTr("Подключиться")
                            enabled: !book.loading && modelData.hasPassword
                            onClicked: book.connectEntry(modelData.id)
                        }
                    }
                }

                Text {
                    anchors.centerIn: parent
                    visible: book.entries.length === 0 && !book.loading
                    text: book.loggedIn ? qsTr("В книге нет записей") : ""
                    color: Theme.textSecondary
                    font.pixelSize: 13
                }
            }
        }

        Text {
            Layout.fillWidth: true
            visible: book.lastError.length > 0
            text: book.lastError
            color: "#ff8fa3"
            font.pixelSize: 12
            wrapMode: Text.WordWrap
        }
    }
}
