#ifndef DIRECTORYITEM_H
#define DIRECTORYITEM_H

#include <QGraphicsItemGroup>

class QModelIndex;

class FileSystemView;

class DirectoryItem : public QGraphicsItemGroup
{
public:
    DirectoryItem(FileSystemView *view, QGraphicsItem *parent, const QModelIndex &index, const QString &name);
    virtual QRectF boundingRect() const;

    void updateView();

protected:
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

private:
    FileSystemView *m_view;
    QString m_path;
    QGraphicsTextItem *m_nameText, *m_detailsText;
};

#endif // DIRECTORYITEM_H
