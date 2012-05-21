import QtQuick 1.1
import com.nokia.meego 1.0

PageStackWindow {
    id: appWindow
    initialPage: fileSystemPage
    showToolBar: false
    showStatusBar: false

    FileSystemPage {
        id: fileSystemPage
    }

    ImagePage {
        id: imagePage
    }

    function showMain(index) {
        appWindow.pageStack.pop(fileSystemPage, true)
    }

    function showImage(index) {
        imagePage.loadImage(index)
        appWindow.pageStack.push(imagePage, {}, true)
    }

}
