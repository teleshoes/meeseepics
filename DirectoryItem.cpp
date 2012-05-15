#include "DirectoryItem.h"
#include "FileSystemView.h"
#include "FileSystemModel.h"

#include <QModelIndex>
#include <QThreadPool>

DirectoryItem::DirectoryItem(FileSystemView *view, const QModelIndex &index, const QString &name)
    : QGraphicsItemGroup(view)
{
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    //setFlag(QGraphicsItem::ItemIsFocusable, true);

    m_path = view->m_model->data(index, FileSystemModel::FilePathRole).toString();

    QGraphicsTextItem *text = new QGraphicsTextItem(name, this);
    text->setDefaultTextColor(view->fontColor());

    QFont font = text->font();
    font.setPixelSize(view->fontPixelSize());
    text->setFont(font);

    addToGroup(text);
}

QRectF DirectoryItem::boundingRect() const
{
    QRectF r = QGraphicsItemGroup::boundingRect();
    r.setWidth(static_cast<FileSystemView*>(parentItem())->width());
    return r;
}

void DirectoryItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItemGroup::mouseReleaseEvent(event);

    // Activate the item. That means change the directory to the choosen directory-item. We do
    // delay the execution cause setDirectory will emit directoryLoaded/rootPathChanged which
    // calls modelReset which deletes our current item. To be sure that happens not at a
    // moment where we still do things with our item afterwards (what would lead to a crash)
    // we just queue the call.
    FileSystemView *view = static_cast<FileSystemView*>(parentItem());
    QMetaObject::invokeMethod(view->m_model, "setDirectory", Qt::QueuedConnection, Q_ARG(QString, m_path));
}
