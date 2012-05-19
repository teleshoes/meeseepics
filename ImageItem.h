#ifndef IMAGEITEM_H
#define IMAGEITEM_H

#include <QObject>
#include <QRunnable>
#include <QMutex>
#include <QGraphicsPixmapItem>
#include <QThreadPool>

class ImageThread : public QObject, public QRunnable
{
    Q_OBJECT
public:
    explicit ImageThread(const QString &path, qreal size = 0);
    virtual ~ImageThread();

    void kill();
    virtual void run();

signals:
    void imageLoaded(const QImage &image);

private:
    QString m_path;
    qreal m_size;
    QMutex m_mutex;

    bool jpegExifReadWord(QIODevice &sdevice, unsigned short *target, bool invert = true);
    bool jpegExifScanloop(QIODevice &jpegFile, unsigned int &tnOffset, unsigned int &tnLength);
    void jpegExifReadThumbnail(QIODevice &jpegFile, QImage &image);
};

class ImageItem : public QObject, public QGraphicsPixmapItem
{
    Q_OBJECT
public:
    explicit ImageItem(QGraphicsItem *parent = 0);
    virtual ~ImageItem();

    static QThreadPool* thumbnailThreadPool();
    static QThreadPool* imageThreadPool();

public slots:
    void loadImage(const QString &path, qreal size = 0, int priority = 0);
    void abortLoadImage();

signals:
    void imageLoaded();

protected slots:
    void setImage(const QImage &image);

private:
    ImageThread *m_thread;
};

#endif // IMAGEITEM_H
