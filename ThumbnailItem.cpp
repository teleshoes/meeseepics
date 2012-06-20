#include "ThumbnailItem.h"
#include "FileSystemView.h"
#include "FileSystemModel.h"

#include <QGraphicsItemGroup>
#include <QGraphicsTextItem>
#include <QFont>
#include <QImageReader>
#include <QApplication>
#include <QModelIndex>
#include <QDateTime>

ThumbnailItem::ThumbnailItem(FileSystemView *view, QGraphicsItem *parent, const QModelIndex &index, qreal size)
    : QGraphicsItemGroup(parent)
    , m_index(index)
    , m_view(view)
    , m_image(0)
    , m_nameText(0)
    , m_detailsText(0)
{
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    //setFlag(QGraphicsItem::ItemIsFocusable, true);

    m_image = new ImageItem(this);
    m_image->setTransformationMode(Qt::FastTransformation);
    addToGroup(m_image);

    QString path = m_view->m_model->data(index, FileSystemModel::FilePathRole).toString();
    m_image->loadImage(path, size);
}

ThumbnailItem::~ThumbnailItem()
{
}

void ThumbnailItem::setDetailedFileList(bool enabled)
{
    if (enabled) {
        if (!m_nameText) {
            m_nameText = new QGraphicsTextItem(this);
            addToGroup(m_nameText);
        }

        m_nameText->setDefaultTextColor(m_view->folderNameFontColor());
        m_nameText->setFont(m_view->folderNameFont());

        QString name = m_view->m_model->data(m_index, FileSystemModel::FileNameRole).toString();
        m_nameText->setPlainText(name);

        if (!m_detailsText) {
            m_detailsText = new QGraphicsTextItem(this);
            addToGroup(m_detailsText);
        }

        m_detailsText->setDefaultTextColor(m_view->folderDetailsFontColor());
        m_detailsText->setFont(m_view->folderDetailsFont());

        QString modified = m_view->m_model->data(m_index, FileSystemModel::FileModifiedRole).toDateTime().toString("yyyy-MM-dd hh:mm:ss");
        QString size = m_view->m_model->data(m_index, FileSystemModel::FileSizeRole).toString();
        QString details = QString("%1   %2").arg(size).arg(modified);
        m_detailsText->setPlainText(details);

    } else {
        delete m_nameText;
        m_nameText = 0;
        delete m_detailsText;
        m_detailsText = 0;
    }
}

void ThumbnailItem::nextThumbnailPosition(int &column, int &row, qreal &x, qreal &y, qreal &tw)
{
    if (m_nameText && m_detailsText) {
        m_nameText->setPos(tw + 5, 0);
        //m_nameText->setTextWidth(boundingRect().width() - m_nameText->pos().x());
        m_detailsText->setPos(m_nameText->pos().x(), m_nameText->boundingRect().bottom() - 8);

        x = 0.0;
        y += tw;
        column = 0;
        ++row;
    } else {
        if (column == m_view->imagesPerRow() - 1) {
            x = 0.0;
            y += tw;
            column = 0;
            ++row;
        } else {
            x += tw;
            ++column;
        }
    }
}

void ThumbnailItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItemGroup::mouseReleaseEvent(event);

    QString path = m_view->m_model->data(m_index, FileSystemModel::FilePathRole).toString();
    static_cast<FileSystemView*>(parentItem()->parentItem())->emitShowImage(path);
}
