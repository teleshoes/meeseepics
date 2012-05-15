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
};

ImageView::ImageView(QDeclarativeItem *parent)
    : QDeclarativeItem(parent)
    , m_imageItem(new ImageViewItem(this))
    , m_totalScaleFactor(1.0)
    , m_proxyModel(0)
{
    setAcceptTouchEvents(true);
    setAcceptedMouseButtons(/* Qt::NoButton | */ Qt::LeftButton | Qt::RightButton | Qt::MiddleButton);
    //setKeepMouseGrab(true);
    setSmooth(true);
    //setTransformOrigin(QDeclarativeItem::Center);
    setFlag(QGraphicsItem::ItemClipsToShape, true);
    setFlag(QGraphicsItem::ItemClipsChildrenToShape, true);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    //setFlag(QGraphicsItem::ItemSendsScenePositionChanges, true);

    m_imageItem->setTransformationMode(Qt::SmoothTransformation);
    m_imageItem->setFlag(QGraphicsItem::ItemIsMovable, true);
    m_imageItem->setFlag(QGraphicsItem::ItemIgnoresParentOpacity, true);

    connect(m_imageItem, SIGNAL(imageLoaded()), this, SLOT(imageLoaded()));
    //m_imageItem->installSceneEventFilter(this);
    //m_imageItem->setAcceptTouchEvents(true);
    //m_imageItem->setAcceptedMouseButtons(/* Qt::NoButton | */ Qt::LeftButton | Qt::RightButton | Qt::MiddleButton);
}

ImageView::~ImageView()
{
}

QRectF ImageView::boundingRect() const
{
    QDeclarativeItem *p = parentItem();
    Q_ASSERT(p);
    if (!p)
        return QDeclarativeItem::boundingRect();
    return p->boundingRect();
}

bool ImageView::setImage(const QModelIndex &index)
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
    m_path = model->data(sourceIndex, FileSystemModel::FilePathRole).toString();
    Q_ASSERT(!m_path.isEmpty());

    qDebug() << "ImageView::setImage path=" << m_path;
    m_imageItem->loadImage(m_path);

    return true;
}

void ImageView::imageLoaded()
{
    zoomToFit();
}

bool ImageView::previousImage()
{
    qDebug() << "ImageView::previousImage";
    QModelIndex index = pathIndex();
    Q_ASSERT(index.isValid());
    int r = index.row();
    index = m_proxyModel->index(r - 1, 0, index.parent());
    if (!index.isValid())
        return false;
    return setImage(index);
}

bool ImageView::nextImage()
{
    qDebug() << "ImageView::nextImage";
    QModelIndex index = pathIndex();
    Q_ASSERT(index.isValid());
    int r = index.row();
    index = m_proxyModel->index(r + 1, 0, index.parent());
    if (!index.isValid())
        return false;
    return setImage(index);
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
    //qreal moveX = (size.width() - width()) / 2.0;
    //qreal moveY = (size.height() - height()) / 2.0;
    m_totalScaleFactor = factor;
    m_imageItem->setScale(m_totalScaleFactor);
    //m_imageItem->setPos(-moveX, -moveY);
}

bool ImageView::sceneEvent(QEvent *event)
{
    switch (event->type()) {
        case QEvent::TouchBegin:
        case QEvent::TouchUpdate:
        case QEvent::TouchEnd: {
            QTouchEvent *touchEvent = static_cast<QTouchEvent *>(event);
            QList<QTouchEvent::TouchPoint> touchPoints = touchEvent->touchPoints();
            if (touchPoints.count() == 1) {
                qDebug()<<"aaaaaaaaaa"    ;
            } else if (touchPoints.count() == 2) {
                const QTouchEvent::TouchPoint &touchPoint0 = touchPoints.first();
                const QTouchEvent::TouchPoint &touchPoint1 = touchPoints.last();
                qreal scaleFactor = QLineF(touchPoint0.pos(), touchPoint1.pos()).length() / QLineF(touchPoint0.startPos(), touchPoint1.startPos()).length();
                if (touchEvent->touchPointStates() & Qt::TouchPointReleased) {
                    m_totalScaleFactor *= scaleFactor;
                    scaleFactor = 1;
                }
                m_imageItem->setScale(m_totalScaleFactor * scaleFactor);
                //m_imageItem->scroll(scaleFactor, scaleFactor);
            }
            return true;
        }
        case QEvent::GraphicsSceneMousePress: {
            QGraphicsSceneMouseEvent *mouseEvent = static_cast<QGraphicsSceneMouseEvent*>(event);
            qDebug()<<"ImageView::sceneEvent GraphicsSceneMousePress screenPos="<<mouseEvent->screenPos()<<"lastScreenPos="<<mouseEvent->lastScreenPos()<<"rotation="<<rotation();
            return true;
        }
        case QEvent::GraphicsSceneMouseRelease: {
            QGraphicsSceneMouseEvent *mouseEvent = static_cast<QGraphicsSceneMouseEvent*>(event);
            QPointF p = mapFromScene(mouseEvent->scenePos());
            qreal mx = p.x();
            qreal my = p.y();
            if (my > boundingRect().height() - 30) {
                emit closeImage();
            } else if (mx < 30) {
                previousImage();
            } else if (mx > boundingRect().width() - 30) {
                nextImage();
            }
            return true;
        }
        case QEvent::GraphicsSceneMouseDoubleClick: {
            QSize size = m_imageItem->pixmap().size();
            bool isZoomToFit = qFuzzyCompare(qMin(width() / size.width(), height() / size.height()), m_totalScaleFactor);
            if (isZoomToFit)
                zoom(1.0);
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
    qDebug() << "ImageView::itemChange change=" << change << "value=" << value << "result=" << result;

    /*
    switch (change) {
        case QGraphicsItem::ItemPositionChange:
        case QGraphicsItem::ItemPositionHasChanged:
        case QGraphicsItem::ItemMatrixChange:
        case QGraphicsItem::ItemTransformChange:
        case QGraphicsItem::ItemTransformHasChanged:
        case QGraphicsItem::ItemRotationChange:
        case QGraphicsItem::ItemRotationHasChanged:
        case QGraphicsItem::ItemScaleChange:
        case QGraphicsItem::ItemScaleHasChanged:
        case QGraphicsItem::ItemTransformOriginPointChange:
        case QGraphicsItem::ItemTransformOriginPointHasChanged:
        default:
            break;
    }
    */

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
    Q_ASSERT(sourceIndex.isValid());
    QModelIndex index = m_proxyModel->mapFromSource(sourceIndex);
    Q_ASSERT(index.isValid());
    return index;
}