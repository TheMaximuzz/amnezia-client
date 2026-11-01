import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Style 1.0

import "./"
import "../Controls2"
import "../Controls2/TextTypes"
import "../Config"
import "../Components"

PageType {
    id: root

    property var values: ({})
    property bool hasTunnelConfig: false

    Component.onCompleted: {
        root.values = LocalInterfaceController.load()
        root.hasTunnelConfig = root.values.hasTunnelConfig
        publicKeyField.textField.text = LocalInterfaceController.publicKey(root.values.privateKey)
    }

    Connections {
        target: LocalInterfaceController

        function onFinished(message) {
            PageController.showNotificationMessage(message)
        }

        function onErrorOccurred(errorMessage) {
            PageController.showErrorMessage(errorMessage)
        }
    }

    FlickableType {
        id: flickable

        anchors.top: parent.top
        anchors.bottom: saveButton.top
        contentHeight: content.implicitHeight + 24

        ColumnLayout {
            id: content

            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right

            spacing: 16

            NavigationBarType {
                Layout.fillWidth: true

                title: qsTr("Local interface")
            }

            NotificationType {
                Layout.fillWidth: true
                Layout.leftMargin: 16
                Layout.rightMargin: 16

                visible: !root.hasTunnelConfig
                text: qsTr("Address, keys and MTU are available when the default server uses AmneziaWG or WireGuard")
            }

            TextFieldType {
                id: addressField

                Layout.fillWidth: true
                Layout.leftMargin: 16
                Layout.rightMargin: 16

                visible: root.hasTunnelConfig
                headerText: qsTr("Address")
                textField.text: root.values.address || ""
            }

            TextFieldType {
                id: dnsField

                Layout.fillWidth: true
                Layout.leftMargin: 16
                Layout.rightMargin: 16

                headerText: qsTr("DNS")
                helperText: qsTr("Used when AmneziaDNS is turned off")
                textField.text: root.values.dns || ""
            }

            TextFieldType {
                id: privateKeyField

                Layout.fillWidth: true
                Layout.leftMargin: 16
                Layout.rightMargin: 16

                visible: root.hasTunnelConfig
                headerText: qsTr("Private key (PrivateKey)")
                helperText: qsTr("A new key stops the connection until the server knows its public key")
                passwordToggle: true
                buttonText: qsTr("Create")
                textField.text: root.values.privateKey || ""

                textField.onTextEdited: publicKeyField.textField.text = LocalInterfaceController.publicKey(textField.text)

                clickedFunc: function() {
                    privateKeyField.textField.text = LocalInterfaceController.generatePrivateKey()
                    publicKeyField.textField.text = LocalInterfaceController.publicKey(privateKeyField.textField.text)
                }
            }

            TextFieldType {
                id: publicKeyField

                Layout.fillWidth: true
                Layout.leftMargin: 16
                Layout.rightMargin: 16

                visible: root.hasTunnelConfig
                headerText: qsTr("Public key derived from the private key")
                textField.readOnly: true
                buttonImageSource: "qrc:/images/controls/copy.svg"

                clickedFunc: function() {
                    GC.copyToClipBoard(publicKeyField.textField.text)
                    PageController.showNotificationMessage(qsTr("Copied"))
                }
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.leftMargin: 16
                Layout.rightMargin: 16

                spacing: 16

                TextFieldType {
                    id: mtuField

                    Layout.fillWidth: true

                    visible: root.hasTunnelConfig
                    headerText: qsTr("MTU")
                    textField.text: root.values.mtu || ""
                    textField.validator: IntValidator { bottom: 576; top: 65535 }
                }

                TextFieldType {
                    id: listenPortField

                    Layout.fillWidth: true

                    headerText: qsTr("Listen port (ListenPort)")
                    textField.placeholderText: qsTr("Automatic")
                    textField.text: root.values.listenPort || ""
                    textField.validator: IntValidator { bottom: 1; top: 65535 }
                }
            }

            SectionHeaderType {
                Layout.fillWidth: true
                Layout.leftMargin: 16
                Layout.rightMargin: 16
                Layout.topMargin: 8

                level: SectionHeaderType.Section
                headerText: qsTr("Scripts")
                descriptionText: qsTr("Commands run with your user rights")
            }

            TextFieldType {
                id: preUpField

                Layout.fillWidth: true
                Layout.leftMargin: 16
                Layout.rightMargin: 16

                headerText: qsTr("Run before connecting")
                textField.text: root.values.preUpScript || ""
            }

            TextFieldType {
                id: postUpField

                Layout.fillWidth: true
                Layout.leftMargin: 16
                Layout.rightMargin: 16

                headerText: qsTr("Run after connecting")
                textField.text: root.values.postUpScript || ""
            }

            TextFieldType {
                id: preDownField

                Layout.fillWidth: true
                Layout.leftMargin: 16
                Layout.rightMargin: 16

                headerText: qsTr("Run before disconnecting")
                textField.text: root.values.preDownScript || ""
            }

            TextFieldType {
                id: postDownField

                Layout.fillWidth: true
                Layout.leftMargin: 16
                Layout.rightMargin: 16

                headerText: qsTr("Run after disconnecting")
                textField.text: root.values.postDownScript || ""
            }

            TextFieldType {
                id: scriptTimeoutField

                Layout.fillWidth: true
                Layout.leftMargin: 16
                Layout.rightMargin: 16

                headerText: qsTr("Script timeout, seconds")
                textField.text: root.values.scriptTimeout || "10"
                textField.validator: IntValidator { bottom: 1; top: 600 }
            }

            SwitchRowType {
                id: allowLocalTrafficSwitch

                Layout.fillWidth: true
                Layout.leftMargin: 16
                Layout.rightMargin: 16

                text: qsTr("Allow local traffic")
                descriptionText: qsTr("Devices in your local network stay reachable while the VPN is on")
                checked: root.values.allowLocalTraffic === true
            }
        }
    }

    ButtonType {
        id: saveButton

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.leftMargin: 16
        anchors.rightMargin: 16
        anchors.bottomMargin: 8 + PageController.safeAreaBottomMargin

        text: qsTr("Save")

        onClicked: {
            if (ConnectionController.isConnected) {
                PageController.showNotificationMessage(qsTr("Unable change settings while there is an active connection"))
                return
            }

            LocalInterfaceController.save({
                address: addressField.textField.text,
                privateKey: privateKeyField.textField.text,
                mtu: mtuField.textField.text,
                dns: dnsField.textField.text,
                listenPort: listenPortField.textField.text,
                preUpScript: preUpField.textField.text,
                postUpScript: postUpField.textField.text,
                preDownScript: preDownField.textField.text,
                postDownScript: postDownField.textField.text,
                scriptTimeout: scriptTimeoutField.textField.text,
                allowLocalTraffic: allowLocalTrafficSwitch.checked
            })
        }
    }
}
