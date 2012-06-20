#include "DirectoryItem.h"
#include "FileSystemView.h"
#include "FileSystemModel.h"

#include <QModelIndex>
#include <QThreadPool>
#include <QDateTime>

DirectoryItem::DirectoryItem(FileSystemView *view, QGraphicsItem *parent, const QModelIndex &index, const QString &name)
    : QGraphicsItemGroup(parent)
    , m_view(view)
{
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    //setFlag(QGraphicsItem::ItemIsFocusable, true);

    m_path = view->m_model->data(index, FileSystemModel::FilePathRole).toString();
    QString modified = view->m_model->data(index, FileSystemModel::FileModifiedRole).toDateTime().toString("yyyy-MM-dd hh:mm:ss");

    m_nameText = new QGraphicsTextItem(name, this);
    m_detailsText = new QGraphicsTextItem(modified, this);
    updateView();
    addToGroup(m_nameText);
    addToGroup(m_detailsText);
}

QRectF DirectoryItem::boundingRect() const
{
    QRectF r = QGraphicsItemGroup::boundingRect();
    //r.setHeight(r.height() + 2);
    FileSystemView *view = static_cast<FileSystemView*>(parentItem()->parentItem());
    r.setWidth(view->width());
    return r;
}

void DirectoryItem::updateView()
{
    m_nameText->setDefaultTextColor(m_view->folderNameFontColor());
    m_nameText->setFont(m_view->folderNameFont());

    m_detailsText->setDefaultTextColor(m_view->folderDetailsFontColor());
    m_detailsText->setFont(m_view->folderDetailsFont());
    m_detailsText->setPos(m_nameText->pos().x(), m_nameText->boundingRect().bottom() - 8);
}

void DirectoryItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItemGroup::mouseReleaseEvent(event);

    // Activate the item. That means change the directory to the choosen directory-item. We do
    // delay the execution cause setDirectory will emit directoryLoaded/rootPathChanged which
    // calls modelReset which deletes our current item. To be sure that happens not at a
    // moment where we still do things with our item afterwards (what would lead to a crash)
    // we just queue the call.
    FileSystemView *view = static_cast<FileSystemView*>(parentItem()->parentItem());
    QMetaObject::invokeMethod(view->m_model, "setDirectory", Qt::QueuedConnection, Q_ARG(QString, m_path));
}
