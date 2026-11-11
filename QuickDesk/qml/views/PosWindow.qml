import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts
import QuickDesk 1.0

// Окно кассы: только состояние подключения и ID станции.
// Подключаться к другим устройствам отсюда нельзя — в POS-сборке процесс
// клиента не запускается, а постоянный пароль приходит с сервера.
ApplicationWindow {
    id: root
    width: 520
    height: 340
    minimumWidth: 520
    minimumHeight: 340
    visible: true
    title: qsTr("QuickDesk — касса")
    color: "#10151d"

    property MainController mainController: MainController {
        id: controller
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 24
        spacing: 10

        Text {
            text: qsTr("Касса")
            color: "#e6ecf5"
            font.pixelSize: 22
            font.bold: true
        }

        Text {
            text: controller.isHostConnected ? qsTr("Подключена к серверу") : qsTr("Нет связи с сервером")
            color: controller.isHostConnected ? "#4ade80" : "#f87171"
            font.pixelSize: 15
        }

        Item { Layout.preferredHeight: 8 }

        Text {
            text: qsTr("ID станции")
            color: "#8a97a8"
            font.pixelSize: 12
        }

        Text {
            text: controller.deviceId && controller.deviceId.length > 0 ? controller.deviceId : "—"
            color: "#e6ecf5"
            font.pixelSize: 18
            font.family: "Consolas"
            font.letterSpacing: 1
        }

        Text {
            Layout.fillWidth: true
            Layout.topMargin: 12
            text: qsTr("Постоянный пароль задаётся администратором. На кассе он не отображается и меняется удалённо.")
            color: "#8a97a8"
            font.pixelSize: 12
            wrapMode: Text.WordWrap
        }

        Item { Layout.fillHeight: true }

        RowLayout {
            Layout.fillWidth: true

            Button {
                text: qsTr("Свернуть")
                onClicked: root.hide()
            }

            Item { Layout.fillWidth: true }

            Text {
                text: "v" + APP_VERSION
                color: "#5b6675"
                font.pixelSize: 11
            }
        }
    }

    onClosing: function(close) {
        close.accepted = false
        root.hide()
    }
}
