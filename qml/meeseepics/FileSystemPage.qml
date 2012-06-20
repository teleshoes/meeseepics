import QtQuick 1.1
import com.nokia.meego 1.0
import FileSystemView 1.0

Page {
    id: fileSystemPage
    anchors.fill: parent

    property string captionText: ''
    property variant fileSystemView: fileSystemViewItem

    Label {
        id: dummyNameLabel
        visible: false
        font.bold: true
    }

    Label {
        id: dummyDetailsLabel
        visible: false
    }

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
        contentHeight: captionItem.height + fileSystemViewItem.implicitHeight
        clip: true

        CaptionItem {
            id: captionItem

            Label {
                id: captionLabel
                parent: captionItem.buttonRow
                //width: parent.width
                width: flickable.width - slideShowButton.width - settingsButton.width
                height: slideShowButton.height
                anchors.verticalCenter: parent.verticalCenter
                color: theme.inverted ? '#ccccff' : '#000099'
                font.weight: Font.Normal
                elide: Text.ElideLeft
                verticalAlignment: Text.AlignVCenter
                text: fileSystemPage.captionText
            }

            Button {
                id: slideShowButton
                parent: captionItem.buttonRow
                anchors.verticalCenter: parent.verticalCenter
                width: height * 1.3
                iconSource: "image://theme/icon-m-toolbar-mediacontrol-play" + (theme.inverted ? "-white" : "")
                onClicked: {
                    var p = loadFile("SlideShowPage.qml")
                    if (p !== null) {
                        appWindow.pageStack.push(p);
                    }
                }
            }

            Button {
                id: settingsButton
                parent: captionItem.buttonRow
                anchors.verticalCenter: parent.verticalCenter
                width: height * 1.3
                iconSource: "image://theme/icon-m-toolbar-settings" + (theme.inverted ? "-white" : "")
                onClicked: {
                    var p = loadFile("SettingsPage.qml")
                    if (p !== null) {
                        appWindow.pageStack.push(p);
                    }
                }
            }
        }

        FileSystemViewItem {
            id: fileSystemViewItem
            width: flickable.width
            height: Math.max(flickable.height - captionItem.height, implicitHeight)
            folderNameFontColor: theme.inverted ? '#ffffff' : '#000000'
            folderNameFont: dummyNameLabel.font
            folderDetailsFontColor: theme.inverted ? '#c0c0c0' : '#636363'
            folderDetailsFont: dummyDetailsLabel.font
            imagesPerRow: appWindow.inPortrait ? appWindow.imagesPerRowInPortrait : appWindow.imagesPerRowInLandscape
            anchors {
                top: captionItem.bottom
                left: parent.left
                right: parent.right
            }
            onShowImage: {
                appWindow.showImage(index)
            }
            onRootPathChanged: {
                fileSystemPage.captionText = path
            }
            onDirectoryLoaded: {
                fileSystemPage.captionText = path
            }
        }
    }

}
