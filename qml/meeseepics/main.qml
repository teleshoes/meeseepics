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

    function showImage(index) {
        appWindow.pageStack.push(imagePage)
        imagePage.setImage(index)
    }

}
