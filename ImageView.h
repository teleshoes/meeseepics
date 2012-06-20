#ifndef IMAGEVIEW_H
#define IMAGEVIEW_H

#include <QDeclarativeItem>

class QModelIndex;
class FileSystemProxyModel;
class ImageViewItem;

class ImageView : public QDeclarativeItem
{
    Q_OBJECT
    Q_PROPERTY(qreal borderSize READ borderSize WRITE setBorderSize)

public:
    explicit ImageView(QDeclarativeItem *parent = 0);
    virtual ~ImageView();
    virtual QRectF boundingRect() const;

    qreal borderSize() const;
    void setBorderSize(qreal size);

public slots:
    void setModel(FileSystemProxyModel *proxyModel);

    bool loadImage(const QModelIndex &index);
    bool previousImage();
    bool nextImage();

    QPointF pos() const;
    void setPos(const QPointF &position);

    qreal zoom() const;
    bool isZoomToFit() const;

    void zoom(qreal factor);
    void zoomToCenter(qreal factor = 1.0);
    void zoomToFit();

    void startSlideShow(bool random, bool loop, int interval);
    void stopSlideShow();

signals:
    void closeImage();

private slots:
    void imageLoaded();
    void slideShowTimeout();

protected:
    virtual bool sceneEvent(QEvent *event);
    virtual void geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry);
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);

private:
    friend class ImageViewItem;

    ImageViewItem *m_imageItem;
    qreal m_totalScaleFactor;
    FileSystemProxyModel *m_proxyModel;
    QRectF m_lastNewGeometry, m_lastOldGeometry;
    QString m_path;
    qreal m_borderSize;

    class SlideShow;
    SlideShow *m_slideShow;

    struct ImageCache {
        QPointF m_pos;
        qreal m_scale;
    };
    QMap<QString, ImageCache*> m_imageCache;

    enum Border { NoBorder, LeftBorder, RightBorder, TopBorder, BottomBorder };
    Border m_borderMousePress;
    Border border(const QPointF &pos) const;

    QModelIndex pathIndex() const;
};

#endif // IMAGEVIEW_H
