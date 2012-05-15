#ifndef IMAGEVIEW_H
#define IMAGEVIEW_H

#include <QDeclarativeItem>

class QModelIndex;
class FileSystemProxyModel;
class ImageViewItem;

class ImageView : public QDeclarativeItem
{
    Q_OBJECT
public:
    explicit ImageView(QDeclarativeItem *parent = 0);
    virtual ~ImageView();
    virtual QRectF boundingRect() const;

public slots:
    bool setImage(const QModelIndex &index);
    bool previousImage();
    bool nextImage();

    void zoom(qreal factor);
    void zoomToFit();

signals:
    void closeImage();

private slots:
    void imageLoaded();

protected:
    virtual bool sceneEvent(QEvent *event);
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);

private:
    friend class ImageViewItem;

    ImageViewItem *m_imageItem;
    qreal m_totalScaleFactor;
    FileSystemProxyModel *m_proxyModel;
    QString m_path;

    QModelIndex pathIndex() const;
};

#endif // IMAGEVIEW_H
