#ifndef DIRECTORYITEM_H
#define DIRECTORYITEM_H

#include <QGraphicsItemGroup>

class QModelIndex;

class FileSystemView;

class DirectoryItem : public QGraphicsItemGroup
{
public:
    DirectoryItem(FileSystemView *view, const QModelIndex &index, const QString &name);
    virtual QRectF boundingRect() const;

protected:
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

private:
    QString m_path;
};

#endif // DIRECTORYITEM_H
