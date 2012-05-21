import QtQuick 1.1
import com.nokia.meego 1.0

Rectangle {
    id: captionItem
    anchors {
        top: parent.top
        left: parent.left
        right: parent.right
    }
    height: captionColumn.height
    color: '#212121'

    property string captionText: ''

    Column {
        id: captionColumn
        x: 0
        width: parent.width
        height: captionLabel.height + 12
        Row {
            id: buttonRow
            anchors {
                left: captionColumn.left
                right: captionColumn.right
                verticalCenter: captionColumn.verticalCenter
            }

            Label {
                id: captionLabel
                width: parent.width
                //width: parent.width - toolsButton.width
                //height: toolsButton.height
                anchors.verticalCenter: parent.verticalCenter
                color: '#ffffff'
                font.weight: Font.Normal
                elide: Text.ElideLeft
                verticalAlignment: Text.AlignVCenter
                text: captionText
            }

            /*
            Button {
                id: toolsButton
                anchors.verticalCenter: parent.verticalCenter
                width: height * 1.3
                checkable: true
                iconSource: "image://theme/icon-m-toolbar-tools" + (theme.inverted ? "-white" : "")
                onClicked: {
                    if (checked) {
                        var p = loadFile("ToolsPage.qml")
                        if (p !== null) {
                            appWindow.pageStack.push(p);
                        }
                    } else {
                        appWindow.showEditorPage();
                    }
                }
            }
            */
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

