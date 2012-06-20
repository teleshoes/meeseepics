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
        contentHeight: contentColumn.height
        clip: true

        Column {
            id: contentColumn
            width: flickable.width
            spacing: 4

            CaptionItem {
                id: captionItem

                Button {
                    id: slideShowButton
                    parent: captionItem.buttonRow
                    anchors.verticalCenter: parent.verticalCenter
                    width: height * 1.3
                    iconSource: "image://theme/icon-m-toolbar-back" + (theme.inverted ? "-white" : "")
                    onClicked: {
                        appWindow.showMain(false);
                        //appWindow.showMain();
                    }
                }
            }

            Column {
                anchors {
                    horizontalCenter: parent.horizontalCenter
                    topMargin: 10
                    bottomMargin: 10
                }
                spacing: 10

                CheckBox {
                    id: randomCheckbox
                    text: qsTr('Random')
                    checked: true
                }

                CheckBox {
                    id: loopCheckbox
                    text: qsTr('Loop')
                    checked: false
                }

                Row {
                    id: intervalRow
                    spacing: 5
                    Label {
                        id: intervalLabel
                        text: qsTr('Interval:')
                        height: intervalEdit.height
                        anchors.verticalCenter: parent.verticalCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    TextField {
                        id: intervalEdit
                        inputMask: "00009"
                        validator: IntValidator { bottom: 1; top: 32768 }
                        text: '5'
                    }
                }
                Button {
                    id: startSlideShow
                    text: qsTr('Start Slideshow')
                    anchors {
                        horizontalCenter: parent.horizontalCenter
                        topMargin: 10
                    }
                    onClicked: {
                        var random = randomCheckbox.checked
                        var loop = loopCheckbox.checked
                        var interval = parseInt(intervalEdit.text) * 1000
                        appWindow.startSlideShow(random, loop, interval)
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
