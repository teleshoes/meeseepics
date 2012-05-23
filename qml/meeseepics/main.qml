import QtQuick 1.1
import com.nokia.meego 1.0

PageStackWindow {
    id: appWindow
    showToolBar: false
    showStatusBar: false
    initialPage: pageStack.currentPage === null || pageStack.currentPage === undefined ? fileSystemPage : undefined

    FileSystemPage {
        id: fileSystemPage
    }

    ImagePage {
        id: imagePage
    }

    function showMain() {
        //appWindow.pageStack.pop(fileSystemPage, true)
        appWindow.pageStack.push(fileSystemPage, {}, true)
    }

    function showImage(index) {
        imagePage.loadImage(index)
        appWindow.pageStack.push(imagePage, {}, true)
    }
}
