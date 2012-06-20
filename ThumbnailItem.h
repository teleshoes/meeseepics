#ifndef THUMBNAILITEM_H
#define THUMBNAILITEM_H

#include "ImageItem.h"

#include <QGraphicsItemGroup>
#include <QPersistentModelIndex>

class FileSystemView;
//class QGraphicsItemGroup;
class QModelIndex;

class ThumbnailItem : public QGraphicsItemGroup
{
public:
    ThumbnailItem(FileSystemView *view, QGraphicsItem *parent, const QModelIndex &index, qreal size);
    virtual ~ThumbnailItem();

    void setDetailedFileList(bool enabled);
    void nextThumbnailPosition(int &column, int &row, qreal &x, qreal &y, qreal &tw);

protected:
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

private:
    QPersistentModelIndex m_index;
    FileSystemView *m_view;
    ImageItem *m_image;
    QGraphicsTextItem *m_nameText;
    QGraphicsTextItem *m_detailsText;

    void loadImage();
};

#endif // THUMBNAILITEM_H
