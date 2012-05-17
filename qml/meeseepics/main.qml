import QtQuick 1.1
import com.nokia.meego 1.0

PageStackWindow {
    id: appWindow
    initialPage: mainPage
    showToolBar: false
    showStatusBar: false

    MainPage {
        id: mainPage
    }

    ImagePage {
        id: imagePage
    }

    function showMain(index) {
        appWindow.pageStack.pop(mainPage, true)
    }

    function showImage(index) {
        imagePage.loadImage(index)
        appWindow.pageStack.push(imagePage, {}, true)
    }

}
