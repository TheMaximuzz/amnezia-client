import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import Style 1.0

import "../Controls2"
import "../Controls2/TextTypes"

ColumnLayout {
    id: root

    property string headerText
    property string descriptionText
    property string placeholderText
    property string emptyText
    property var model
    property string textRole: "url"
    property string subtitleRole: ""
    property bool showAppButtons: false
    property bool showMoveButtons: false
    property int listHeight: 160
    property int currentIndex: -1

    signal addRequested(string text)
    signal removeRequested(int index)
    signal moveRequested(int index, int offset)
    signal addFolderRequested()
    signal addFileRequested()
    signal addProcessRequested()

    spacing: 8

    SectionHeaderType {
        Layout.fillWidth: true

        level: SectionHeaderType.Section
        headerText: root.headerText
        descriptionText: root.descriptionText
    }

    RowLayout {
        spacing: 4

        IconButtonType {
            visible: root.showAppButtons
            size: IconButtonType.Small
            icon.source: "qrc:/images/controls/folder-search-2.svg"
            ToolTip.visible: hovered
            ToolTip.text: qsTr("Add folder")
            onClicked: root.addFolderRequested()
        }

        IconButtonType {
            visible: root.showAppButtons
            size: IconButtonType.Small
            icon.source: "qrc:/images/controls/file-check-2.svg"
            ToolTip.visible: hovered
            ToolTip.text: qsTr("Add program file")
            onClicked: root.addFileRequested()
        }

        IconButtonType {
            visible: root.showAppButtons
            size: IconButtonType.Small
            icon.source: "qrc:/images/controls/monitor.svg"
            ToolTip.visible: hovered
            ToolTip.text: qsTr("Choose a running process")
            onClicked: root.addProcessRequested()
        }

        IconButtonType {
            visible: root.showMoveButtons
            enabled: root.currentIndex > 0
            size: IconButtonType.Small
            icon.source: "qrc:/images/controls/chevron-up.svg"
            ToolTip.visible: hovered
            ToolTip.text: qsTr("Move up")
            onClicked: {
                root.moveRequested(root.currentIndex, -1)
                root.currentIndex -= 1
            }
        }

        IconButtonType {
            visible: root.showMoveButtons
            enabled: root.currentIndex >= 0 && root.currentIndex < listView.count - 1
            size: IconButtonType.Small
            icon.source: "qrc:/images/controls/chevron-down.svg"
            ToolTip.visible: hovered
            ToolTip.text: qsTr("Move down")
            onClicked: {
                root.moveRequested(root.currentIndex, 1)
                root.currentIndex += 1
            }
        }

        IconButtonType {
            enabled: root.currentIndex >= 0 && root.currentIndex < listView.count
            size: IconButtonType.Small
            icon.source: "qrc:/images/controls/x-circle.svg"
            ToolTip.visible: hovered
            ToolTip.text: qsTr("Remove selected")
            onClicked: {
                root.removeRequested(root.currentIndex)
                root.currentIndex = -1
            }
        }
    }

    TextFieldType {
        id: inputField

        Layout.fillWidth: true

        textField.placeholderText: root.placeholderText
        buttonImageSource: "qrc:/images/controls/plus.svg"
        buttonJoined: true
        rightButtonClickedOnEnter: true

        clickedFunc: function() {
            if (inputField.textField.text.trim() !== "") {
                root.addRequested(inputField.textField.text.trim())
                inputField.textField.text = ""
            }
        }
    }

    Rectangle {
        Layout.fillWidth: true
        Layout.preferredHeight: root.listHeight

        radius: 12
        color: AmneziaStyle.color.bgField
        border.width: 1
        border.color: AmneziaStyle.color.borderFaint
        clip: true

        ListView {
            id: listView

            anchors.fill: parent
            anchors.margins: 4

            model: root.model
            currentIndex: root.currentIndex
            boundsBehavior: Flickable.StopAtBounds

            ScrollBar.vertical: ScrollBarType {}

            delegate: ItemDelegate {
                id: row

                required property int index
                required property var model

                width: listView.width
                height: 36

                background: Rectangle {
                    radius: 8
                    color: row.index === root.currentIndex ? AmneziaStyle.color.bgStateLayerAccent
                         : row.hovered ? AmneziaStyle.color.bgStateLayer : AmneziaStyle.color.transparent
                    border.width: row.index === root.currentIndex ? 1 : 0
                    border.color: AmneziaStyle.color.borderAccent
                }

                contentItem: RowLayout {
                    spacing: 12

                    CaptionTextType {
                        Layout.fillWidth: true

                        text: row.model[root.textRole]
                        color: AmneziaStyle.color.textPrimary
                        elide: Text.ElideMiddle
                    }

                    CaptionTextType {
                        visible: root.subtitleRole !== "" && text !== ""
                        Layout.maximumWidth: listView.width * 0.4

                        text: root.subtitleRole !== "" ? row.model[root.subtitleRole] : ""
                        color: AmneziaStyle.color.textTertiary
                        elide: Text.ElideRight
                    }
                }

                onClicked: root.currentIndex = row.index
            }
        }

        CaptionTextType {
            anchors.centerIn: parent

            visible: listView.count === 0
            text: root.emptyText
            color: AmneziaStyle.color.textQuaternary
        }
    }
}
