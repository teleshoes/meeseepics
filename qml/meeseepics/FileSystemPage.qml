import QtQuick 1.1
import com.nokia.meego 1.0
import FileSystemView 1.0

Page {
    anchors.fill: parent

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
        color: '#000000'
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
        }

        FileSystemViewItem {
            id: fileSystemViewItem
            width: flickable.width
            height: Math.max(flickable.height - captionItem.height, implicitHeight)
            folderNameFontColor: '#ffffff'
            folderNameFont: dummyNameLabel.font
            folderDetailsFontColor: '#c0c0c0'
            folderDetailsFont: dummyDetailsLabel.font
            imagesPerRow: appWindow.inPortrait ? 4 : 7
            anchors {
                top: captionItem.bottom
                left: parent.left
                right: parent.right
            }
            onShowImage: {
                appWindow.showImage(index)
            }
            onRootPathChanged: {
                captionItem.captionText = path
            }
            onDirectoryLoaded: {
                captionItem.captionText = path
            }
        }
    }

}
