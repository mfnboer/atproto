import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
    property string user
    property string password
    property string host

    title: qsTr("Login")
    width: parent.width
    standardButtons: Dialog.Ok
    modal: true

    GridLayout {
        columns: 2
        anchors.fill: parent

        Label {
            text: qsTr("User:")
        }
        TextField {
            id: userField
            Layout.fillWidth: true
            focus: true
        }

        Label {
            text: qsTr("Password:")
        }
        TextField {
            id: passwordField
            Layout.fillWidth: true
            echoMode: TextInput.Password
        }

        Label {
            text: qsTr("Host:")
        }
        TextField {
            id: hostField
            Layout.fillWidth: true
            text: "bsky.social"
        }
    }

    onAccepted: {
        user = userField.text
        password = passwordField.text
        host = hostField.text
    }
}
