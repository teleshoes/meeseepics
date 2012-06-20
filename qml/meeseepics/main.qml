import QtQuick 1.1
import com.nokia.meego 1.0

PageStackWindow {
    id: appWindow
    showToolBar: false
    showStatusBar: false
    initialPage: pageStack.currentPage === null || pageStack.currentPage === undefined ? fileSystemPage : undefined

    property bool invertTheme: true
    property bool detailedFileList: false
    property int imagesPerRowInPortrait: 4
    property int imagesPerRowInLandscape: 7

    onImagesPerRowInLandscapeChanged: {
        Settings.setValue("imagesPerRowInLandscape", imagesPerRowInLandscape)
    }
    onImagesPerRowInPortraitChanged: {
        Settings.setValue("imagesPerRowInPortrait", imagesPerRowInPortrait)
    }

    FileSystemPage {
        id: fileSystemPage
    }

    ImagePage {
        id: imagePage
        Component.onCompleted: {
            console.log("proxyModel:", fileSystemPage.fileSystemView.proxyModel);
            imagePage.imageView.setModel(fileSystemPage.fileSystemView.proxyModel)
        }
    }

    function loadFile(file) {
        var component = Qt.createComponent(file)
        if (component.status != Component.Ready) {
            console.log("Error loading component:", component.errorString());
            return null;
        }
        var obj = component.createObject(parent, {});
        console.log("Loaded component:", component, " and created an intance of object:", obj);
        return obj
    }

    function showMain(immediate) {
        appWindow.pageStack.pop(fileSystemPage, immediate === undefined || immediate === true)
    }

    function showImage(index, immediate) {
        imagePage.loadImage(index)
        appWindow.pageStack.push(imagePage, {}, immediate === undefined || immediate === true)
    }

    function startSlideShow(random, loop, interval, immediate) {
        imagePage.startSlideShow(random, loop, interval)
        appWindow.pageStack.push(imagePage, {}, immediate === undefined || immediate === true)
    }

    function reloadFileSystem() {
        fileSystemPage.fileSystemView.modelReset()
    }

    onInvertThemeChanged: {
        theme.inverted = appWindow.invertTheme
        Settings.setValue("invertTheme", appWindow.invertTheme)
    }
    onDetailedFileListChanged: {
        fileSystemPage.fileSystemView.detailedFileList = appWindow.detailedFileList
        Settings.setValue("detailedFileList", appWindow.detailedFileList)
    }

    function isNumber(obj) {
      return obj !== null && !isNaN(obj-0);
    }

    Component.onCompleted: {
        var inverttheme = Settings.value("invertTheme")
        appWindow.invertTheme = inverttheme === undefined || inverttheme === null || inverttheme
        if (theme.inverted !== appWindow.invertTheme)
            theme.inverted = appWindow.invertTheme

        var detailedfilelist = Settings.value("detailedFileList")
        appWindow.detailedFileList = detailedfilelist !== undefined && detailedfilelist !== null && detailedfilelist
        if (fileSystemPage.fileSystemView.detailedFileList !== appWindow.detailedFileList)
            fileSystemPage.fileSystemView.detailedFileList = appWindow.detailedFileList

        var p = Settings.value("imagesPerRowInPortrait")
        if (isNumber(p))
            appWindow.imagesPerRowInPortrait = p

        var l = Settings.value("imagesPerRowInLandscape")
        if (isNumber(l))
            appWindow.imagesPerRowInLandscape = l
    }
}
