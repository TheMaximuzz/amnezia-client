import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import SortFilterProxyModel 0.2

import Style 1.0

import "../Controls2"
import "../Controls2/TextTypes"

SheetType {
    id: root

    property int selectedIndex: -1

    expandedHeight: windowHeight * 0.9

    onAboutToShow: {
        root.selectedIndex = -1
        WindowsSplitTunnelingController.refreshProcesses(allUsersCheckBox.checked)
    }

    Item {
        anchors.fill: parent

        ColumnLayout {
            id: header

            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.topMargin: 12
            anchors.leftMargin: 16
            anchors.rightMargin: 16

            spacing: 12

            SectionHeaderType {
                Layout.fillWidth: true

                headerText: qsTr("Choose a process")
                descriptionText: qsTr("The selected program will not use the VPN")
            }

            RowLayout {
                Layout.fillWidth: true

                spacing: 12

                SearchFieldType {
                    id: filterField

                    Layout.fillWidth: true

                    placeholderText: qsTr("Filter")
                    onTextChanged: root.selectedIndex = -1
                }

                CheckBoxType {
                    id: allUsersCheckBox

                    text: qsTr("Show processes of all users")

                    onToggled: {
                        root.selectedIndex = -1
                        WindowsSplitTunnelingController.refreshProcesses(checked)
                    }
                }
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.leftMargin: 12
                Layout.rightMargin: 12

                spacing: 12

                CaptionTextType {
                    Layout.preferredWidth: processList.width * 0.25
                    text: qsTr("Process name")
                    color: AmneziaStyle.color.textTertiary
                }

                CaptionTextType {
                    Layout.preferredWidth: processList.width * 0.15
                    text: qsTr("User")
                    color: AmneziaStyle.color.textTertiary
                }

                CaptionTextType {
                    Layout.fillWidth: true
                    text: qsTr("Process file path")
                    color: AmneziaStyle.color.textTertiary
                }
            }
        }

        Rectangle {
            anchors.top: header.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: footer.top
            anchors.margins: 16

            radius: 12
            color: AmneziaStyle.color.bgField
            border.width: 1
            border.color: AmneziaStyle.color.borderFaint
            clip: true

            ListView {
                id: processList

                anchors.fill: parent
                anchors.margins: 4

                boundsBehavior: Flickable.StopAtBounds
                ScrollBar.vertical: ScrollBarType {}

                model: SortFilterProxyModel {
                    id: filteredProcesses

                    sourceModel: RunningProcessesModel
                    filters: AnyOf {
                        RegExpFilter {
                            roleName: "processName"
                            pattern: ".*" + filterField.text + ".*"
                            caseSensitivity: Qt.CaseInsensitive
                        }
                        RegExpFilter {
                            roleName: "processPath"
                            pattern: ".*" + filterField.text + ".*"
                            caseSensitivity: Qt.CaseInsensitive
                        }
                    }
                }

                delegate: ItemDelegate {
                    id: processRow

                    required property int index
                    required property string processName
                    required property string processUser
                    required property string processPath

                    width: processList.width
                    height: 36

                    background: Rectangle {
                        radius: 8
                        color: processRow.index === root.selectedIndex ? AmneziaStyle.color.bgStateLayerAccent
                             : processRow.hovered ? AmneziaStyle.color.bgStateLayer : AmneziaStyle.color.transparent
                        border.width: processRow.index === root.selectedIndex ? 1 : 0
                        border.color: AmneziaStyle.color.borderAccent
                    }

                    contentItem: RowLayout {
                        spacing: 12

                        CaptionTextType {
                            Layout.preferredWidth: processList.width * 0.25
                            text: processRow.processName
                            color: AmneziaStyle.color.textPrimary
                            elide: Text.ElideRight
                        }

                        CaptionTextType {
                            Layout.preferredWidth: processList.width * 0.15
                            text: processRow.processUser
                            color: AmneziaStyle.color.textTertiary
                            elide: Text.ElideRight
                        }

                        CaptionTextType {
                            Layout.fillWidth: true
                            text: processRow.processPath
                            color: AmneziaStyle.color.textSecondary
                            elide: Text.ElideMiddle
                        }
                    }

                    onClicked: root.selectedIndex = processRow.index
                    onDoubleClicked: {
                        root.selectedIndex = processRow.index
                        root.addSelectedProcess()
                    }
                }
            }

            CaptionTextType {
                anchors.centerIn: parent

                visible: processList.count === 0
                text: qsTr("No processes found")
                color: AmneziaStyle.color.textQuaternary
            }
        }

        ColumnLayout {
            id: footer

            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.leftMargin: 16
            anchors.rightMargin: 16
            anchors.bottomMargin: Math.max(16, PageController.safeAreaBottomMargin)

            spacing: 12

            CheckBoxType {
                id: wholeFolderCheckBox

                Layout.fillWidth: true

                checked: true
                text: qsTr("Exclude the whole folder of the process")
                descriptionText: qsTr("All programs in this folder and its subfolders will bypass the VPN, including ones that appear after updates")
            }

            RowLayout {
                Layout.fillWidth: true

                spacing: 12

                ButtonType {
                    Layout.fillWidth: true

                    variant: ButtonType.Secondary
                    text: qsTr("Cancel")

                    onClicked: root.close()
                }

                ButtonType {
                    Layout.fillWidth: true

                    enabled: root.selectedIndex >= 0
                    text: qsTr("Choose process")

                    onClicked: root.addSelectedProcess()
                }
            }
        }
    }

    function addSelectedProcess() {
        if (root.selectedIndex < 0) {
            return
        }

        WindowsSplitTunnelingController.addProcess(filteredProcesses.get(root.selectedIndex, "processPath"),
                                                   wholeFolderCheckBox.checked)
        root.close()
    }
}
