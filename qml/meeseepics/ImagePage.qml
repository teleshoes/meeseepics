import QtQuick 1.1
import com.nokia.meego 1.0
import ImageView 1.0

Page {
    id: imagePage

    property variant imageView: imageViewItem

    Rectangle {
        id: backgroundColor
        anchors.fill: parent
        color: '#000000'
    }

    ImageViewItem {
        id: imageViewItem
        anchors.fill: parent
        onCloseImage: appWindow.showMain()

        /*
        onVisibleChanged: {
            if (!imageView.visible)
                imageView.source = ''
        }
        */
    }

    function loadImage(index) {
        imageView.loadImage(index)
    }

    function startSlideShow(random, loop, interval) {
        imageView.startSlideShow(random, loop, interval)
    }
}
