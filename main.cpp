#include <QtGui/QApplication>
#include "qmlapplicationviewer.h"

#include "FileSystemView.h"
#include "ImageView.h"
#include "Settings.h"

#include <QtDeclarative>
#include <QDeclarativeEngine>
#include <QDeclarativeContext>

Q_DECL_EXPORT int main(int argc, char *argv[])
{
    QScopedPointer<QApplication> app(createApplication(argc, argv));

    QmlApplicationViewer viewer;
    QDeclarativeContext *ctxt = viewer.rootContext();

    qmlRegisterType<FileSystemView>("FileSystemView", 1, 0, "FileSystemViewItem");
    qmlRegisterType<ImageView>("ImageView", 1, 0, "ImageViewItem");

    ctxt->setContextProperty("Settings", Settings::instance());

    viewer.setOrientation(QmlApplicationViewer::ScreenOrientationAuto);
    viewer.setMainQmlFile(QLatin1String("qml/meeseepics/main.qml"));
    viewer.showExpanded();

    int result = app->exec();

    // Call that explicit since the dtor of Settings may not be called...
    Settings::instance()->saveChanges();

    return result;
}
