#ifndef FILESYSTEMVIEW_H
#define FILESYSTEMVIEW_H

#include <QObject>
#include <QDeclarativeItem>

class QModelIndex;

class FileSystemModel;
class FileSystemProxyModel;
class DirectoryItem;
class ThumbnailItem;

class FileSystemView : public QDeclarativeItem
{
    Q_OBJECT
    Q_PROPERTY(QColor fontColor READ fontColor WRITE setFontColor)
    Q_PROPERTY(int fontPixelSize READ fontPixelSize WRITE setFontPixelSize)
    Q_PROPERTY(int imagesPerRow READ imagesPerRow WRITE setImagesPerRow)
    Q_PROPERTY(int thumbnailThreadCount READ thumbnailThreadCount WRITE setThumbnailThreadCount)

public:
    explicit FileSystemView(QDeclarativeItem *parent = 0);

    QColor fontColor() const;
    void setFontColor(const QColor &color);

    int fontPixelSize() const;
    void setFontPixelSize(int size);

    int imagesPerRow() const;
    void setImagesPerRow(int number);

    int thumbnailThreadCount() const;
    void setThumbnailThreadCount(int count);

signals:
    void showImage(const QModelIndex &index);

private slots:
    void modelReset();
    void modelArrange();
    bool emitShowImage(const QString &path);
    bool emitShowImage(const QModelIndex &index);

protected:
    virtual void geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry);

private:
    friend class DirectoryItem;
    friend class ThumbnailItem;

    FileSystemModel *m_model;
    FileSystemProxyModel *m_proxyModel;
    QRectF m_lastNewGeometry, m_lastOldGeometry;
    QColor m_fontColor;
    int m_fontPixelSize;
    int m_imagesPerRow;

    bool inPortrait() const;
};

#endif // FILESYSTEMVIEW_H
