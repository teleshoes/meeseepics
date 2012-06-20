import QtQuick 1.1
import com.nokia.meego 1.0

Page {
    id: slideShowPage

    Rectangle {
        id: backgroundColor
        anchors.fill: parent
        color: theme.inverted ? '#000000' : '#ffffff'
    }

    Flickable {
        id: flickable
        anchors {
            top: parent.top
            bottom: parent.bottom
            left: parent.left
            right: parent.right
        }
        contentHeight: flickableColumn.height
        clip: true

        Column {
            id: flickableColumn
            width: flickable.width
            spacing: 4

            CaptionItem {
                id: captionItem

                Button {
                    id: slideShowButton
                    parent: captionItem.buttonRow
                    //anchors.verticalCenter: parent.verticalCenter
                    width: height * 1.3
                    iconSource: "image://theme/icon-m-toolbar-back" + (theme.inverted ? "-white" : "")
                    onClicked: {
                        appWindow.showMain(false)
                        //appWindow.showMain();
                    }
                }
            }

            Column {
                id: contentColumn
                anchors {
                    left: flickableColumn.left
                    right: flickableColumn.right
                    topMargin: 10
                    bottomMargin: 10
                    leftMargin: 10
                    rightMargin: 10
                }
                spacing: 10

                Label {
                    text: qsTr('Theme:')
                }

                ButtonRow {
                    id: invertThemeButtons
                    width: parent.width
                    checkedButton: darkButton
                    Button {
                        id: lightButton
                        width: parent.width / 2
                        text: qsTr('Light')
                    }
                    Button {
                        id: darkButton
                        width: parent.width / 2
                        text: qsTr('Dark')
                    }
                    function valueChangedFunction() {
                        appWindow.invertTheme = (invertThemeButtons.checkedButton !== lightButton)
                    }
                    Component.onCompleted: {
                        invertThemeButtons.checkedButton = (appWindow.invertTheme ? darkButton : lightButton)
                        invertThemeButtons.checkedButtonChanged.connect(valueChangedFunction)
                    }
                }

                Label {
                    text: qsTr('File List:')
                }

                ButtonRow {
                    id: fileListButtons
                    width: parent.width
                    Button {
                        id: compactButton
                        text: qsTr('Compact')
                        width: parent.width / 2
                    }
                    Button {
                        id: detailedButton
                        text: qsTr('Detailed')
                        width: parent.width / 2
                    }
                    function valueChangedFunction() {
                        appWindow.detailedFileList = (fileListButtons.checkedButton !== compactButton)
                        if (imagesPerRowEdit.checkedButton === biggestButton)
                            imagesPerRowEdit.valueChangedFunction()
                    }
                    Component.onCompleted: {
                        fileListButtons.checkedButton = (appWindow.detailedFileList ? detailedButton : compactButton)
                        fileListButtons.checkedButtonChanged.connect(valueChangedFunction)
                    }
                }

                Label {
                    text: qsTr('Thumbnail Size:')
                }

                ButtonRow {
                    id: imagesPerRowEdit
                    width: parent.width

                    function valueChangedFunction() {
                        if (imagesPerRowEdit.checkedButton === smallButton) {
                            appWindow.imagesPerRowInPortrait = 5
                            appWindow.imagesPerRowInLandscape = 9
                        } else if (imagesPerRowEdit.checkedButton === normalButton) {
                            appWindow.imagesPerRowInPortrait = 4
                            appWindow.imagesPerRowInLandscape = 7
                        } else if (imagesPerRowEdit.checkedButton === bigButton) {
                            appWindow.imagesPerRowInPortrait = 3
                            appWindow.imagesPerRowInLandscape = 5
                        } else if (imagesPerRowEdit.checkedButton === biggestButton) {
                            appWindow.imagesPerRowInPortrait = appWindow.detailedFileList ? 2 : 1
                            appWindow.imagesPerRowInLandscape = appWindow.detailedFileList ? 2 : 1
                        } else
                            console.debug('Invalid checkedButton=' + imagesPerRowEdit.checkedButton + ' for imagesPerRowEdit.onValueChanged')

                        appWindow.reloadFileSystem()
                    }

                    Button {
                        id: smallButton
                        text: qsTr('Small')
                        width: parent.width / 4
                    }
                    Button {
                        id: normalButton
                        text: qsTr('Medium')
                        width: parent.width / 4
                    }
                    Button {
                        id: bigButton
                        text: qsTr('Big')
                        width: parent.width / 4
                    }
                    Button {
                        id: biggestButton
                        text: qsTr('Biggest')
                        width: parent.width / 4
                    }
                    Component.onCompleted: {
                        if (appWindow.imagesPerRowInPortrait === 1 || appWindow.imagesPerRowInPortrait === 2) {
                            imagesPerRowEdit.checkedButton = biggestButton
                        } else if (appWindow.imagesPerRowInPortrait === 3) {
                            imagesPerRowEdit.checkedButton = bigButton
                        } else if (appWindow.imagesPerRowInPortrait === 4) {
                            imagesPerRowEdit.checkedButton = normalButton
                        } else if (appWindow.imagesPerRowInPortrait === 5) {
                            imagesPerRowEdit.checkedButton = smallButton
                        } else
                            console.debug('Invalid value ' + appWindow.imagesPerRowInPortrait + ' for imagesPerRowEdit.onCompleted')

                        imagesPerRowEdit.checkedButtonChanged.connect(valueChangedFunction)
                    }
                }
            }
        }
    }

    onVisibleChanged: {
        if (!slideShowPage.visible) {
            destroy(slideShowPage)
        }
    }
}
