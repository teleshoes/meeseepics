import QtQuick 1.1
import com.nokia.meego 1.0
import ImageView 1.0

Page {
    id: imagePage

    Rectangle {
        id: backgroundColor
        anchors.fill: parent
        color: '#000000'
    }

    ImageViewItem {
        id: imageView
        anchors.fill: parent

        onCloseImage: {
            appWindow.pageStack.pop()
        }

        /*
        onVisibleChanged: {
            if (!imageView.visible)
                imageView.source = ''
        }
        */
    }

    function setImage(index) {
        imageView.setImage(index)
    }

}
