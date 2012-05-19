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
        anchors.fill: parent
        contentHeight: fileSystemViewItem.implicitHeight
        FileSystemViewItem {
            id: fileSystemViewItem
            width: flickable.width
            height: Math.max(flickable.height, implicitHeight)
            folderNameFontColor: '#ffffff'
            folderNameFont: dummyNameLabel.font
            folderDetailsFontColor: '#c0c0c0'
            folderDetailsFont: dummyDetailsLabel.font
            imagesPerRow: appWindow.inPortrait ? 4 : 7
            anchors {
                top: parent.top
                left: parent.left
                right: parent.right
            }
            onShowImage: {
                appWindow.showImage(index)
            }
        }
    }

}
