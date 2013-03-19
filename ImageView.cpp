#include "ImageView.h"
#include "ImageItem.h"
#include "FileSystemModel.h"
#include "FileSystemProxyModel.h"

#include <QModelIndex>
#include <QGraphicsScene>
#include <QImageReader>
#include <QEvent>
#include <QTouchEvent>
#include <QGraphicsSceneMouseEvent>
#include <QTime>
#include <QTimer>
#include <qmdisplaystate.h>

class ImageView::SlideShow
{
public:
    QTimer m_slideShowTimer;
    bool m_slideShowRandom;
    bool m_slideShowLoop;
    QStringList m_slideShowPaths;
    MeeGo::QmDisplayState m_displayState;

    explicit SlideShow()
        : m_slideShowRandom(true)
        , m_slideShowLoop(true)
    {
        qsrand(QTime::currentTime().msec());
        m_slideShowTimer.setInterval(5000);
    }

    void stop() {
        qDebug() << "SlideShow::stop";
        m_slideShowTimer.stop();
        m_slideShowPaths.clear();
        m_displayState.cancelBlankingPause();
    }

    void pause() {
        qDebug() << "SlideShow::pause";
        m_slideShowTimer.stop();
    }

    void start(int interval = -1) {
        qDebug() << "SlideShow::start interval=" << interval << "slideShowTimerInterval=" << m_slideShowTimer.interval();
        if (interval >= 0)
            m_slideShowTimer.setInterval(interval);
        m_slideShowTimer.start();
        m_displayState.setBlankingPause();
    }

    QString nextPath() {
        if (m_slideShowPaths.isEmpty()) {
            qDebug() << "SlideShow::nextPath empty";
            return QString();
        }
        int pos = 0;
        if (m_slideShowRandom) {
            pos = qrand() % m_slideShowPaths.count();
        }
        QString path = m_slideShowPaths.takeAt(pos);
        qDebug() << "SlideShow::nextPath pos=" << pos << "count=" << m_slideShowPaths.count() << "path=" << path;
        return path;
    }
};

class ImageViewItem : public ImageItem
{
public:
    explicit ImageViewItem(ImageView *itemView) : ImageItem(itemView) {}

protected:
    virtual bool sceneEvent(QEvent *event)
    {
        switch (event->type()) {
            case QEvent::TouchBegin:
            case QEvent::TouchUpdate:
            case QEvent::TouchEnd:
            case QEvent::Gesture:
            case QEvent::GraphicsSceneMousePress:
            case QEvent::GraphicsSceneMouseRelease:
            case QEvent::GraphicsSceneMouseDoubleClick:
                return static_cast<ImageView*>(parentItem())->sceneEvent(event);
            default: {
                break;
            }
        }
        return QGraphicsPixmapItem::sceneEvent(event);
    }

    /*
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value)
    {
        QVariant result = QGraphicsPixmapItem::itemChange(change, value);
        qDebug() << "ImageViewItem::itemChange change=" << change << "value=" << value << "result=" << result;
        return result;
    }
    */
};

ImageView::ImageView(QDeclarativeItem *parent)
    : QDeclarativeItem(parent)
    , m_imageItem(new ImageViewItem(this))
    , m_totalScaleFactor(1.0)
    , m_proxyModel(0)
    , m_borderSize(35)
    , m_borderMousePress(NoBorder)
    , m_slideShow(new ImageView::SlideShow())
{
    setAcceptTouchEvents(true);
    setAcceptedMouseButtons(Qt::LeftButton | Qt::RightButton | Qt::MiddleButton);
    setSmooth(true);
    setFlag(QGraphicsItem::ItemClipsToShape, true);
    setFlag(QGraphicsItem::ItemClipsChildrenToShape, true);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);

    m_imageItem->setTransformationMode(Qt::SmoothTransformation);
    m_imageItem->setFlag(QGraphicsItem::ItemIgnoresParentOpacity, true);
    connect(m_imageItem, SIGNAL(imageLoaded()), this, SLOT(imageLoaded()));

    connect(&m_slideShow->m_slideShowTimer, SIGNAL(timeout()), this, SLOT(slideShowTimeout()));
}

ImageView::~ImageView()
{
    delete m_slideShow;
    m_slideShow = 0;

    qDeleteAll(m_imageCache);
}

QRectF ImageView::boundingRect() const
{
    QDeclarativeItem *p = parentItem();
    return p ? p->boundingRect() : QDeclarativeItem::boundingRect();
}

qreal ImageView::borderSize() const
{
    return m_borderSize;
}

void ImageView::setBorderSize(qreal size)
{
    m_borderSize = size;
}

void ImageView::setModel(FileSystemProxyModel *proxyModel)
{
    m_path.clear();

    m_proxyModel = proxyModel;
    FileSystemModel* model = dynamic_cast<FileSystemModel*>(m_proxyModel ? m_proxyModel->sourceModel() : 0);
    Q_ASSERT(m_proxyModel);
    Q_ASSERT(model);
}

bool ImageView::loadImage(const QModelIndex &index)
{
    Q_ASSERT(index.isValid());

    m_proxyModel = dynamic_cast<FileSystemProxyModel*>(const_cast<QAbstractItemModel*>(index.model()));
    Q_ASSERT(m_proxyModel);
    if (!m_proxyModel)
        return false;

    FileSystemModel* model = dynamic_cast<FileSystemModel*>(m_proxyModel->sourceModel());
    Q_ASSERT(model);

    QModelIndex sourceIndex = m_proxyModel->mapToSource(index);
    Q_ASSERT(sourceIndex.isValid());
    QString oldPath = m_path;
    m_path = model->data(sourceIndex, FileSystemModel::FilePathRole).toString();
    Q_ASSERT(!m_path.isEmpty());

    if (!oldPath.isEmpty()) {
        QFileInfo oldFi(oldPath);
        QFileInfo newFi(m_path);
        if (oldFi.absolutePath() != newFi.absolutePath()) {
            qDeleteAll(m_imageCache);
            m_imageCache.clear();
        } else {
            QMap<QString, ImageCache*>::Iterator it = m_imageCache.find(oldPath);
            ImageCache *cache = 0;
            if (it == m_imageCache.constEnd()) {
                if (!isZoomToFit()) {
                    cache = new ImageCache();
                    m_imageCache.insert(oldPath, cache);
                }
            } else {
                if (isZoomToFit()) {
                    delete it.value();
                    m_imageCache.erase(it);
                } else {
                    cache = it.value();
                }
            }
            if (cache) {
                cache->m_pos = m_imageItem->pos();
                cache->m_scale = m_imageItem->scale();
            }
        }
    }

    qDebug() << "ImageView::doLoadImage path=" << m_path;
    qreal size = 0;
    int priority = QThread::HighestPriority;
    m_imageItem->loadImage(m_path, size, priority);

    return true;
}

void ImageView::imageLoaded()
{
    QMap<QString, ImageCache*>::ConstIterator it = m_imageCache.constFind(m_path);
    if (it == m_imageCache.constEnd()) {
        zoomToFit();
    } else {
        ImageCache *cache = it.value();
        m_imageItem->setPos(cache->m_pos);
        m_imageItem->setScale(cache->m_scale);
        m_totalScaleFactor = cache->m_scale;
    }
}

bool ImageView::previousImage()
{
    qDebug() << "ImageView::previousImage";
    Q_ASSERT(m_proxyModel);
    QModelIndex index = pathIndex();
    Q_ASSERT(index.isValid());
    if (!m_proxyModel || !index.isValid())
        return false;
    int r = index.row();
    index = m_proxyModel->index(r - 1, 0, index.parent());
    if (!index.isValid())
        return false;
    return loadImage(index);
}

bool ImageView::nextImage()
{
    qDebug() << "ImageView::nextImage";
    Q_ASSERT(m_proxyModel);
    QModelIndex index = pathIndex();
    Q_ASSERT(index.isValid());
    if (!m_proxyModel || !index.isValid())
        return false;
    int r = index.row();
    index = m_proxyModel->index(r + 1, 0, index.parent());
    if (!index.isValid())
        return false;
    return loadImage(index);
}

qreal ImageView::zoom() const
{
    return m_totalScaleFactor;
}

bool ImageView::isZoomToFit() const
{
    QSize size = m_imageItem->pixmap().size();
    return qFuzzyCompare(qMin(width() / size.width(), height() / size.height()), m_totalScaleFactor);
}

QPointF ImageView::pos() const
{
    return m_imageItem->pos();
}

void ImageView::setPos(const QPointF &position)
{
    m_imageItem->setPos(position.x(), position.y());
}

void ImageView::zoom(qreal factor)
{
    m_totalScaleFactor = factor;
    m_imageItem->setScale(m_totalScaleFactor);
}

void ImageView::zoomToFit()
{
    QSize size = m_imageItem->pixmap().size();
    qreal factor = qMin(width() / size.width(), height() / size.height());
    zoomToCenter(factor);
}

void ImageView::zoomToCenter(qreal factor)
{
    zoom(factor);
    QRectF r = m_imageItem->mapRectToParent(m_imageItem->boundingRect());
    QSizeF s = boundingRect().size();
    qreal x = (s.width() - r.width()) / 2;
    qreal y = (s.height() - r.height()) / 2;
    m_imageItem->setPos(x, y);
}

void ImageView::startSlideShow(bool random, bool loop, int interval)
{
    m_slideShow->stop();

    Q_ASSERT(m_proxyModel);
    FileSystemModel *model = m_proxyModel ? dynamic_cast<FileSystemModel*>(m_proxyModel->sourceModel()) : 0;
    Q_ASSERT(model);
    if (!model)
        return;

    bool instantly = false;

    if (m_path.isEmpty()) {
        QModelIndex dirIndex = m_proxyModel->mapFromSource(model->directoryIndex());
        if (!dirIndex.isValid()) {
            qDebug()<<"ImageView::startSlideShow Invalid directory="<<model->directory();
            return;
        }
        int count = m_proxyModel->rowCount(dirIndex);
        for(int r = 0; r < count; ++r) {
            QModelIndex index = m_proxyModel->index(r, 0, dirIndex);
            if (!index.isValid() || m_proxyModel->data(index, FileSystemModel::FileIsDirRole).toBool())
                continue;
            m_path = m_proxyModel->data(index, FileSystemModel::FilePathRole).toString();
            if (!m_path.isEmpty())
                break;
        }
        if (m_path.isEmpty()) {
            qDebug()<<"ImageView::startSlideShow No files in directory="<<model->directory();
            return;
        }
        instantly = true;
    }

    QModelIndex parent = m_proxyModel->parent(pathIndex());
    int count = m_proxyModel->rowCount(parent);
    qDebug()<<"0........ count="<<count<<m_proxyModel->data(parent, FileSystemModel::FilePathRole).toString()<<"m_path="<<m_path;
    for(int r = 0; r < count; ++r) {
        QModelIndex index = m_proxyModel->index(r, 0, parent);

        bool isDir = m_proxyModel->data(index, FileSystemModel::FileIsDirRole).toBool();
        if (isDir) {
            continue;
        }

        QString path = m_proxyModel->data(index, FileSystemModel::FilePathRole).toString();
        if (!path.isEmpty())
            m_slideShow->m_slideShowPaths.append(path);
    }

    m_slideShow->m_slideShowRandom = random;
    m_slideShow->m_slideShowLoop = loop;
    m_slideShow->m_slideShowTimer.setInterval(interval);

    qDebug() << "ImageView::startSlideShow random=" << random << "loop=" << loop << "interval=" << interval << "pathCount=" << m_slideShow->m_slideShowPaths.count();

    connect(this, SIGNAL(closeImage()), this, SLOT(stopSlideShow()));

    if (instantly)
        slideShowTimeout();
    else
        m_slideShow->start();
}

void ImageView::stopSlideShow()
{
    disconnect(this, SIGNAL(closeImage()), this, SLOT(stopSlideShow()));

    m_slideShow->stop();
}

void ImageView::slideShowTimeout()
{
    qDebug() << "ImageView::slideShowTimeout";
    m_slideShow->pause();
    do {
        QString path = m_slideShow->nextPath();
        if (path.isNull()) {
            if (m_slideShow->m_slideShowLoop) {
                qDebug() << "ImageView::slideShowTimeout finished. Restarting...";
                startSlideShow(m_slideShow->m_slideShowRandom, m_slideShow->m_slideShowLoop, m_slideShow->m_slideShowTimer.interval());
            } else {
                qDebug() << "ImageView::slideShowTimeout finished";
            }
            return; // everything done.
        }

        Q_ASSERT(m_proxyModel);
        FileSystemModel *model = m_proxyModel ? dynamic_cast<FileSystemModel*>(m_proxyModel->sourceModel()) : 0;
        Q_ASSERT(model);
        if (model) {
            QModelIndex index = model->directoryIndex(path);
            Q_ASSERT(index.isValid());
            if (index.isValid()) {
                QModelIndex proxyIndex = m_proxyModel->mapFromSource(index);
                Q_ASSERT(proxyIndex.isValid());
                if (loadImage(proxyIndex)) {
                    qDebug() << "ImageView::slideShowTimeout loaded image=" << path;
                    break; // leave while-loop and wait for next timeout
                } else {
                    qDebug() << "ImageView::slideShowTimeout Failed loadeding image=" << path;
                    continue; // try loading next image
                }
            }
        }
    } while(true);
    m_slideShow->start();
}

void ImageView::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    //qDebug() << "ImageView::geometryChanged newGeometry=" << newGeometry << "oldGeometry=" << oldGeometry << "rotation=" << rotation();
    QDeclarativeItem::geometryChanged(newGeometry, oldGeometry);
    if (newGeometry == oldGeometry || (m_lastNewGeometry == newGeometry && m_lastOldGeometry == oldGeometry))
        return;
    m_lastNewGeometry = newGeometry;
    m_lastOldGeometry = oldGeometry;
    qreal x = (newGeometry.width() - oldGeometry.width()) / 2;
    qreal y = (newGeometry.height() - oldGeometry.height()) / 2;
    m_imageItem->moveBy(x, y);
}

bool ImageView::sceneEvent(QEvent *event)
{
    switch (event->type()) {
        case QEvent::TouchBegin:
        case QEvent::TouchUpdate:
        case QEvent::TouchEnd: {
            QTouchEvent *touchEvent = static_cast<QTouchEvent *>(event);
            QList<QTouchEvent::TouchPoint> touchPoints = touchEvent->touchPoints();
            if (touchPoints.count() == 1 /* && m_borderMousePress == NoBorder */ ) {
                const QTouchEvent::TouchPoint &touchPoint0 = touchPoints.first();
                qreal x = touchPoint0.pos().x() - touchPoint0.lastPos().x();
                qreal y = touchPoint0.pos().y() - touchPoint0.lastPos().y();
                QRectF r1 = boundingRect();
                QRectF r2 = m_imageItem->mapRectToParent(m_imageItem->boundingRect());
                if (r2.width() <= r1.width()) {
                    x = 0;
                } else if (x > 0) {
                    x = r2.left() >= r1.left() ? 0 : qMin(x, r1.left() - r2.left());
                } else {
                    x = r2.right() >= r1.right() ? qMin(x, r2.right() - r1.right()) : 0;
                }
                if (r2.height() <= r1.height()) {
                    y = 0;
                } else if (y > 0) {
                    y = r2.top() >= r1.top() ? 0 : qMin(y, r1.top() - r2.top());
                } else {
                    y = r2.bottom() >= r1.bottom() ? qMin(y, r2.bottom() - r1.bottom()) : 0;
                }
                m_imageItem->moveBy(x, y);
            } else if (touchPoints.count() == 2) {
                const QTouchEvent::TouchPoint &touchPoint0 = touchPoints.first();
                const QTouchEvent::TouchPoint &touchPoint1 = touchPoints.last();
                qreal scaleFactor = QLineF(touchPoint0.pos(), touchPoint1.pos()).length() / QLineF(touchPoint0.startPos(), touchPoint1.startPos()).length();
                if (touchEvent->touchPointStates() & Qt::TouchPointReleased) {
                    m_totalScaleFactor *= scaleFactor;
                    scaleFactor = 1;
                }
                QRectF r1 = boundingRect();
                QRectF r2 = m_imageItem->mapRectToParent(m_imageItem->boundingRect());
                m_imageItem->setScale(m_totalScaleFactor * scaleFactor);
                QRectF r3 = m_imageItem->mapRectToParent(m_imageItem->boundingRect());
                qreal x = (r2.width() - r3.width()) / 2;
                qreal y = (r2.height() - r3.height()) / 2;
                if (x > 0 && r1.width() < r3.width()) {
                    if (r3.left() + x > r1.left()) {
                        x = qMax<qreal>(0, r3.left() - r1.left());
                    } else if (r3.right() + x < r1.right()) {
                        x = qMax<qreal>(0, r1.right() - r3.right());
                    }
                }
                if (y > 0 && r1.height() < r3.height()) {
                    if (r3.top() + y > r1.top()) {
                        y = qMax<qreal>(0, r3.top() - r1.top());
                    } else if (r3.bottom() + y < r1.bottom()) {
                        y = qMax<qreal>(0, r1.bottom() - r3.bottom());
                    }
                }
                m_imageItem->moveBy(x, y);
            }
            return true;
        }
        case QEvent::GraphicsSceneMousePress: {
            QGraphicsSceneMouseEvent *mouseEvent = static_cast<QGraphicsSceneMouseEvent*>(event);
            QPointF p = mapFromScene(mouseEvent->scenePos());
            m_borderMousePress = border(p);
            return true;
        }
        case QEvent::GraphicsSceneMouseRelease: {
            QGraphicsSceneMouseEvent *mouseEvent = static_cast<QGraphicsSceneMouseEvent*>(event);
            QPointF p = mapFromScene(mouseEvent->scenePos());
            Border b = border(p);
            if (b == NoBorder || b != m_borderMousePress)
                return false;
            if (b == LeftBorder)
                previousImage();
            else if (b == RightBorder)
                nextImage();
            else if (b == TopBorder || b == BottomBorder)
                emit closeImage();
            return true;
        }
        case QEvent::GraphicsSceneMouseDoubleClick: {
            if (isZoomToFit())
                zoomToCenter(m_totalScaleFactor * 2.0);
            else
                zoomToFit();
            return true;
        }
        default: {
            break;
        }
    }
    return QDeclarativeItem::sceneEvent(event);
}

QVariant ImageView::itemChange(GraphicsItemChange change, const QVariant &value)
{
    QVariant result = QDeclarativeItem::itemChange(change, value);
    //qDebug() << "ImageView::itemChange change=" << change << "value=" << value << "result=" << result;
    return result;
}

QModelIndex ImageView::pathIndex() const
{
    Q_ASSERT(m_proxyModel);
    if (!m_proxyModel)
        return QModelIndex();
    FileSystemModel* model = dynamic_cast<FileSystemModel*>(m_proxyModel->sourceModel());
    Q_ASSERT(model);
    QModelIndex sourceIndex = model->index(m_path);
    Q_ASSERT_X(sourceIndex.isValid(), __FUNCTION__, qPrintable(QString("Invalid path=%1").arg(m_path)));
    QModelIndex index = m_proxyModel->mapFromSource(sourceIndex);
    Q_ASSERT(index.isValid());
    return index;
}

ImageView::Border ImageView::border(const QPointF &pos) const
{
    if (pos.y() < m_borderSize)
        return TopBorder;
    if (pos.y() > boundingRect().height() - m_borderSize)
        return BottomBorder;
    if (pos.x() < m_borderSize)
        return LeftBorder;
    if (pos.x() > boundingRect().width() - m_borderSize)
        return RightBorder;
    return NoBorder;
}
