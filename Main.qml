import QtCore
import QtQuick
import QtQuick.Controls.Material
import QtQuick.Window
import Qt5Compat.GraphicalEffects
import atproto

Window {
    width: 640
    height: 480
    visible: true
    title: qsTr("Hello World")

    Image {
        id: banner
        width: parent.width
        source: test.banner
        fillMode: Image.PreserveAspectFit
    }

    Rectangle {
        x: parent.x + 10
        Rectangle {
            id: avatarBack
            y: banner.y + banner.height - 52
            width: 104
            height: 104
            radius: 52
            color: "white"
        }

        Image {
            id: avatar
            x: avatarBack.x + 2
            y: avatarBack.y + 2
            width: 100
            height: 100
            source: test.avatar
            fillMode: Image.PreserveAspectFit
            visible: false
        }
        Rectangle {
            id: mask
            width: 100
            height: 100
            radius: 50
            visible: false
        }
        OpacityMask {
            anchors.fill: avatar
            source: avatar
            maskSource: mask
        }

        Label {
            id: displayName
            width: parent.width
            anchors.top: avatarBack.bottom
            font.pointSize: 27
            text: test.displayName
        }

        Label {
            width: parent.width
            anchors.top: displayName.bottom
            wrapMode: Text.WordWrap
            text: test.description
        }
    }

    Login {
        id: loginDialog
        anchors.centerIn: parent
        onAccepted: {
            test.login(user, password, host)
        }
    }

    ATProtoTest {
        id: test
    }

    Component.onCompleted: {
        loginDialog.open()
    }
}
