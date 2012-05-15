#include <QtGui/QApplication>
#include "qmlapplicationviewer.h"

#include "FileSystemView.h"
#include "ImageView.h"

#include <QtDeclarative>
#include <QDeclarativeEngine>
#include <QDeclarativeContext>

Q_DECL_EXPORT int main(int argc, char *argv[])
{
    QScopedPointer<QApplication> app(createApplication(argc, argv));

    QmlApplicationViewer viewer;
    //QDeclarativeContext *ctxt = viewer.rootContext();

    qmlRegisterType<FileSystemView>("FileSystemView", 1, 0, "FileSystemViewItem");
    qmlRegisterType<ImageView>("ImageView", 1, 0, "ImageViewItem");

    viewer.setOrientation(QmlApplicationViewer::ScreenOrientationAuto);
    viewer.setMainQmlFile(QLatin1String("qml/meeseepics/main.qml"));
    viewer.showExpanded();

    return app->exec();
}
