import QtQuick 1.1
import com.nokia.meego 1.0

Rectangle {
    id: captionItem
    anchors {
        top: parent.top
        left: parent.left
        right: parent.right
    }
    //height: Math.max(captionColumn.height, backgroundImage.height)
    height: captionColumn.height

    property variant captionColumn: cptncolumn
    property variant buttonRow: btnrow

    //color: theme.inverted ? '#000033' : '#cccccc'
    Image {
        id: backgroundImage
        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
        }
        height: captionItem.height
        source: "image://theme/meegotouch-toolbar-" + (appWindow.inPortrait ? "portrait" : "landscape") + (theme.inverted ? "-inverted" : "") + "-background"
    }

    Column {
        id: cptncolumn
        x: 0
        width: parent.width
        //height: captionLabel.height + 12
        Row {
            id: btnrow
            width: parent.width
        }
    }

    /*
    function currentPageChanged() {
        toolsButton.checked = appWindow.pageStack.currentPage !== appWindow.fileSystemPage
    }

    Component.onCompleted: {
        appWindow.pageStack.currentPageChanged.connect(currentPageChanged)
        currentPageChanged()
    }
    */
}

