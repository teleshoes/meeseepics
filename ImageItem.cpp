#include "ImageItem.h"

#include <QThreadPool>
#include <QImageReader>
#include <QFileInfo>
#include <QDir>
#include <QFile>
#include <QBuffer>
#include <QCryptographicHash>
#include <QDebug>

#include <QTime>

/*
// For the explanation of the trick, check out:
// http://www.virtualdub.org/blog/pivot/entry.php?id=116
// http://www.compuphase.com/graphic/scale3.htm
#define AVG(a,b)  ( ((((a)^(b)) & 0xfefefefeUL) >> 1) + ((a)&(b)) )
QImage halfSized(const QImage &source)
{
    QImage dest(source.size() * 0.5, QImage::Format_ARGB32_Premultiplied);
    const quint32 *src = reinterpret_cast<const quint32*>(source.bits());
    int sx = source.bytesPerLine() >> 2;
    int sx2 = sx << 1;
    quint32 *dst = reinterpret_cast<quint32*>(dest.bits());
    int dx = dest.bytesPerLine() >> 2;
    int ww = dest.width();
    int hh = dest.height();
    for (int y = hh; y; --y, dst += dx, src += sx2) {
        const quint32 *p1 = src;
        const quint32 *p2 = src + sx;
        quint32 *q = dst;
        for (int x = ww; x; --x, q++, p1 += 2, p2 += 2)
            *q = AVG(AVG(p1[0], p1[1]), AVG(p2[0], p2[1]));
    }
    return dest;
}
*/

// Exif defines
#define JPEG_SOI 0xffd8
#define JPEG_SOS 0xffda
#define JPEG_EOI 0xffd9
#define JPEG_APP1 0xffe1

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
    QImage image;
    if (m_size > 0) {
        static QString thumbnailDir = QDir::homePath() + QDir::separator() + QLatin1String(".thumbnails") + QDir::separator() + QLatin1String("grid");
        QByteArray hash = QCryptographicHash::hash(QString("file://%1").arg(m_path).toUtf8(), QCryptographicHash::Md5);
        QFileInfo fi(thumbnailDir, hash.toHex() + QLatin1String(".jpeg"));
        if (fi.exists()) {
            QImageReader reader(fi.absoluteFilePath());
            image = reader.read();
        } else {
            QFile file(m_path);
            jpegExifReadThumbnail(file, image);
        }
    }
    if (m_size < 0)
        return;
    if (image.isNull()) {
        QImageReader reader(m_path);
        image = reader.read();
    }
    if (m_size > 0) {
        // Looks as the idea to optimize down-scaling doesnt really optimize according to my
        // measurements. Maybe Qt::FastTransformation got optimized to use that algo already?
        /*
        int doubleSize = m_size * 2;
        if (image.width() >= doubleSize && image.height() >= doubleSize) {
            int ww = image.width() >> 1;
            int hh = image.height() >> 1;
            image = image.copy(0, 0, ww << 1, hh << 1);
            //do {
                image = halfSized(image);
            //} while(image.width() >= doubleSize && image.height() >= doubleSize);
        }
        */

        int w = image.width();
        int h = image.height();
        if (w > h) {
            image = image.copy((w - h) / 2, 0, h, h);
        } else if (w < h) {
            image = image.copy(0, (h - w) / 2, w, w);
        }
        image = image.scaled(QSize(m_size, m_size), Qt::IgnoreAspectRatio, Qt::FastTransformation);
    }
    if (m_size < 0)
        return;
    emit imageLoaded(image);
}

bool ImageThread::jpegExifReadWord(QIODevice &sdevice, unsigned short *target, bool invert)
{
    unsigned short t;
    if (sdevice.read((char*)&t, 2) != 2)
        return false;
    *target = invert ? (((t & 255) << 8) | ((t >> 8) & 255)) : t;
    return true;
}

bool ImageThread::jpegExifScanloop(QIODevice &jpegFile, unsigned int &tnOffset, unsigned int &tnLength) {
    while (1) {
        unsigned short tagid, tagLength;
        if (!jpegExifReadWord(jpegFile, &tagid))
            return false;
        if (tagid == JPEG_EOI || tagid == JPEG_SOS)
            break;
        if (!jpegExifReadWord(jpegFile, &tagLength))
            return false;
        if (tagid == JPEG_APP1) {
            char str[6];
            jpegFile.read(str, 6);
            int basepos = jpegFile.pos();
            unsigned short tifhead[2];
            for (int h = 0; h < 2; h++)
                if (!jpegExifReadWord(jpegFile, &tifhead[h]))
                    return false;
            if (tifhead[0] != 0x4949)
                return false;
            while (1) {
                unsigned int offset;
                jpegFile.read((char*)&offset, 4);
                if (offset == 0)
                    break;
                jpegFile.seek(basepos + offset);
                unsigned short fields;
                if (!jpegExifReadWord(jpegFile, &fields, false))
                    return false;
                while (fields > 0) {
                    char ifdentry[12];
                    jpegFile.read(ifdentry, 12);
                    unsigned short tagnumber = (((unsigned short)ifdentry[0]) | (unsigned short)ifdentry[1] << 8);
                    if (tagnumber == 0x0201) {
                        memcpy(&tnOffset, ifdentry + 8, 4);
                        tnOffset += basepos;
                    } else if (tagnumber == 0x0202)
                        memcpy(&tnLength, ifdentry + 8, 4);
                    --fields;
                    if (tnOffset != 0 && tnLength != 0)
                        return true;
                }
            }
            return false;
        }
        jpegFile.seek(jpegFile.pos() + tagLength - 2);
    }
    return false;
}

void ImageThread::jpegExifReadThumbnail(QIODevice &jpegFile, QImage &image)
{
    if (!jpegFile.open(QIODevice::ReadOnly))
        return;
    unsigned short jpegId;
    if (!jpegExifReadWord(jpegFile, &jpegId)) {
        jpegFile.close();
        return;
    }
    if (jpegId!= JPEG_SOI) {
        jpegFile.close();
        return;
    }
    unsigned int tnOffset = 0;
    unsigned int tnLength = 0;
    if (!jpegExifScanloop(jpegFile, tnOffset, tnLength)) {
        jpegFile.close();
        return;
    }
    jpegFile.seek(tnOffset);
    QByteArray tnArray = jpegFile.read(tnLength);
    QBuffer buf(&tnArray, 0);
    QImageReader reader(&buf);
    reader.setAutoDetectImageFormat(false);
    reader.setFormat("jpg");
    image = reader.read();
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

class ThumbnailThreadPool : public QThreadPool
{
public:
    ThumbnailThreadPool() : QThreadPool() { setMaxThreadCount(4); }
};

class ImageThreadPool : public QThreadPool
{
public:
    ImageThreadPool() : QThreadPool() { setMaxThreadCount(3); }
};

Q_GLOBAL_STATIC(ThumbnailThreadPool, s_thumbnailThreadPool)
Q_GLOBAL_STATIC(ImageThreadPool, s_imageThreadPool)

QThreadPool* ImageItem::thumbnailThreadPool()
{
    return s_thumbnailThreadPool();
}

QThreadPool* ImageItem::imageThreadPool()
{
    return s_imageThreadPool();
}

void ImageItem::loadImage(const QString &path, qreal size, int priority)
{
    //qDebug() << "ImageItem::loadImage path=" << path;

    //setPixmap(QPixmap());

    if (m_thread) {
        disconnect(m_thread, SIGNAL(imageLoaded(QImage)), this, SLOT(setImage(QImage)));
        m_thread->kill();
    }
    m_thread = new ImageThread(path, size);
    connect(m_thread, SIGNAL(imageLoaded(QImage)), this, SLOT(setImage(QImage)));

    bool highPrio = priority >= QThread::HighPriority && priority <= QThread::TimeCriticalPriority;
    QThreadPool *tp = highPrio ? imageThreadPool() : thumbnailThreadPool();
    tp->start(m_thread, priority);
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
    //qDebug() << "ImageItem::setImage imageIsNull=" << image.isNull() << "imageRect=" << image.rect();

    if (m_thread) {
        m_thread->kill();
        m_thread->deleteLater();
        m_thread = 0;
    }

    QPixmap pixmap = QPixmap::fromImage(image);
    setPixmap(pixmap);

    emit imageLoaded();
}
