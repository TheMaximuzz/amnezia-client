import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

import Style 1.0

import "./"
import "../Controls2"
import "../Controls2/TextTypes"
import "../Config"
import "../Components"

PageType {
    id: root

    property bool pageEnabled

    Component.onCompleted: {
        if (ConnectionController.isConnected) {
            PageController.showNotificationMessage(qsTr("Cannot change split tunneling settings during active connection"))
            root.pageEnabled = false
        } else if (ServersUiController.isDefaultServerDefaultContainerHasSplitTunneling) {
            PageController.showNotificationMessage(qsTr("Default server does not support split tunneling function"))
            root.pageEnabled = false
        } else {
            root.pageEnabled = true
        }
    }

    Connections {
        target: WindowsSplitTunnelingController

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
        contentHeight: content.implicitHeight + 32

        ColumnLayout {
            id: content

            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right

            spacing: 24

            NavigationBarType {
                Layout.fillWidth: true

                title: qsTr("Split tunneling")
            }

            BodyTextType {
                Layout.fillWidth: true
                Layout.leftMargin: 16
                Layout.rightMargin: 16

                text: qsTr("Choose which traffic goes through the VPN and which goes directly to the Internet")
                color: AmneziaStyle.color.textSecondary
            }

            SplitTunnelingListSection {
                Layout.fillWidth: true
                Layout.leftMargin: 16
                Layout.rightMargin: 16

                enabled: root.pageEnabled

                headerText: qsTr("Apps outside the tunnel")
                descriptionText: qsTr("These programs and everything they start bypass the VPN")
                placeholderText: qsTr("Path to a program or folder")
                emptyText: qsTr("No filters")
                model: AppSplitTunnelingModel
                textRole: "appPath"
                showAppButtons: true
                showMoveButtons: true

                onAddRequested: function(text) { WindowsSplitTunnelingController.addApp(text) }
                onRemoveRequested: function(index) { WindowsSplitTunnelingController.removeApp(index) }
                onMoveRequested: function(index, offset) { WindowsSplitTunnelingController.moveApp(index, offset) }
                onAddFolderRequested: appFolderDialog.open()
                onAddProcessRequested: processPicker.open()
                onAddFileRequested: {
                    var fileName = SystemController.getFileName(qsTr("Open executable file"),
                                                                qsTr("Executable files (*.exe)"))
                    if (fileName !== "") {
                        WindowsSplitTunnelingController.addApp(fileName)
                    }
                }
            }

            SplitTunnelingListSection {
                Layout.fillWidth: true
                Layout.leftMargin: 16
                Layout.rightMargin: 16

                enabled: root.pageEnabled

                headerText: qsTr("Tunneled networks and addresses")
                descriptionText: qsTr("When the list is empty, all traffic goes through the VPN")
                placeholderText: qsTr("Address, network or domain")
                emptyText: "0.0.0.0/0, ::/0"
                model: TunneledSitesModel
                subtitleRole: "ip"
                listHeight: 120

                onAddRequested: function(text) { WindowsSplitTunnelingController.addTunneledSite(text) }
                onRemoveRequested: function(index) { WindowsSplitTunnelingController.removeTunneledSite(index) }
            }

            SplitTunnelingListSection {
                Layout.fillWidth: true
                Layout.leftMargin: 16
                Layout.rightMargin: 16

                enabled: root.pageEnabled

                headerText: qsTr("Networks and addresses outside the tunnel")
                descriptionText: qsTr("Traffic to these addresses goes directly to the Internet")
                placeholderText: qsTr("Address, network or domain")
                emptyText: qsTr("No filters")
                model: ExcludedSitesModel
                subtitleRole: "ip"

                onAddRequested: function(text) { WindowsSplitTunnelingController.addExcludedSite(text) }
                onRemoveRequested: function(index) { WindowsSplitTunnelingController.removeExcludedSite(index) }
            }

            ListItemType {
                Layout.fillWidth: true
                Layout.leftMargin: 16
                Layout.rightMargin: 16

                enabled: root.pageEnabled

                text: qsTr("Russian addresses directly")
                subtitle: IpSplitTunnelingController.ruSubnetsStatus

                onClicked: {
                    ruSubnetsSwitch.toggle()
                    ruSubnetsSwitch.toggled()
                }

                SwitchType {
                    id: ruSubnetsSwitch

                    focusPolicy: Qt.NoFocus

                    checked: IpSplitTunnelingController.isRuSubnetsBypassEnabled

                    onToggled: {
                        IpSplitTunnelingController.toggleRuSubnetsBypass(checked)
                        WindowsSplitTunnelingController.applySiteLists()
                    }
                }
            }

            ListItemType {
                Layout.fillWidth: true
                Layout.leftMargin: 16
                Layout.rightMargin: 16

                visible: IpSplitTunnelingController.isRuSubnetsBypassEnabled
                enabled: root.pageEnabled && !IpSplitTunnelingController.isRuSubnetsUpdating

                text: qsTr("Update the list of Russian addresses now")
                trailingIconSource: "qrc:/images/controls/refresh-cw.svg"

                onClicked: IpSplitTunnelingController.updateRuSubnets()
            }
        }
    }

    FolderDialog {
        id: appFolderDialog

        title: qsTr("Select a folder to exclude from VPN")

        onAccepted: WindowsSplitTunnelingController.addAppFolder(appFolderDialog.selectedFolder)
    }

    ProcessPickerDrawer {
        id: processPicker
    }
}
