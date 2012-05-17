#include "ImageItem.h"

#include <QThreadPool>
#include <QImageReader>
#include <QDebug>

ImageThread::ImageThread(const QString &path, qreal size)
    : QObject()
    , QRunnable()
    , m_path(path)
    , m_size(size)
{
    setAutoDelete(false);
}

ImageThread::~ImageThread()
{
}

void ImageThread::kill()
{
    m_size = -1;
}

void ImageThread::run()
{
    {
        QMutexLocker locker(&m_mutex);
        Q_UNUSED(locker);
        if (m_size < 0)
            return;
    }
    QImageReader reader(m_path);
    QImage image = reader.read();
    if (m_size > 0)
        image = image.scaled(QSize(m_size, m_size) /*, Qt::KeepAspectRatio*/);
    emit imageLoaded(image);
}

ImageItem::ImageItem(QGraphicsItem *parent)
    : QObject()
    , QGraphicsPixmapItem(parent)
    , m_thread(0)
{
    setShapeMode(QGraphicsPixmapItem::BoundingRectShape);
    setTransformationMode(Qt::SmoothTransformation);
}

ImageItem::~ImageItem()
{
    if (m_thread) {
        ImageThread *t = m_thread;
        m_thread = 0;
        t->kill();
    }
}

class ImageThreadPool : public QThreadPool
{
public:
    ImageThreadPool()
        : QThreadPool()
    {
        setMaxThreadCount(4);
    }
};

Q_GLOBAL_STATIC(ImageThreadPool, s_imageThreadPool)

QThreadPool* ImageItem::threadPool()
{
    return s_imageThreadPool();
}

QRectF ImageItem::boundingRect() const
{
    QRectF r = QGraphicsPixmapItem::boundingRect();
//if (m_size > 0) r.setSize(QSizeF(m_size, m_size));
    return r;
}

void ImageItem::loadImage(const QString &path, qreal size, int priority)
{
    qDebug() << "ImageItem::loadImage path=" << path;

    //setPixmap(QPixmap());

    if (m_thread) {
        disconnect(m_thread, SIGNAL(imageLoaded(QImage)), this, SLOT(setImage(QImage)));
        m_thread->kill();
    }
    m_thread = new ImageThread(path, size);
    connect(m_thread, SIGNAL(imageLoaded(QImage)), this, SLOT(setImage(QImage)));

    QThreadPool *tp = threadPool();
    //bool reserved = priority >= QThread::HighPriority && priority <= QThread::TimeCriticalPriority;
    //if (reserved) tp->reserveThread();
    tp->start(m_thread, priority);
    //if (reserved) tp->releaseThread();
}

void ImageItem::abortLoadImage()
{
    if (m_thread) {
        disconnect(m_thread, SIGNAL(imageLoaded(QImage)), this, SLOT(setImage(QImage)));
        m_thread->kill();
        m_thread = 0;
    }
}

void ImageItem::setImage(const QImage &image)
{
    qDebug() << "ImageItem::setImage imageIsNull=" << image.isNull() << "imageRect=" << image.rect();

    if (m_thread) {
        delete m_thread;
        m_thread = 0;
    }

    QPixmap pixmap = QPixmap::fromImage(image);
    setPixmap(pixmap);

    emit imageLoaded();
}
