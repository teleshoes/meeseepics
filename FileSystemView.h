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

    Q_PROPERTY(QColor folderNameFontColor READ folderNameFontColor WRITE setFolderNameFontColor)
    Q_PROPERTY(QFont folderNameFont READ folderNameFont WRITE setFolderNameFont)

    Q_PROPERTY(QColor folderDetailsFontColor READ folderDetailsFontColor WRITE setFolderDetailsFontColor)
    Q_PROPERTY(QFont folderDetailsFont READ folderDetailsFont WRITE setFolderDetailsFont)

    Q_PROPERTY(int imagesPerRow READ imagesPerRow WRITE setImagesPerRow)
    Q_PROPERTY(int thumbnailThreadCount READ thumbnailThreadCount WRITE setThumbnailThreadCount)
    Q_PROPERTY(int imageThreadCount READ imageThreadCount WRITE setImageThreadCount)

public:
    explicit FileSystemView(QDeclarativeItem *parent = 0);

    QColor folderNameFontColor() const;
    void setFolderNameFontColor(const QColor &color);

    QFont folderNameFont() const;
    void setFolderNameFont(const QFont &font);

    QColor folderDetailsFontColor() const;
    void setFolderDetailsFontColor(const QColor &color);

    QFont folderDetailsFont() const;
    void setFolderDetailsFont(const QFont &font);

    int imagesPerRow() const;
    void setImagesPerRow(int number);

    int thumbnailThreadCount() const;
    void setThumbnailThreadCount(int count);

    int imageThreadCount() const;
    void setImageThreadCount(int count);

public slots:
    void setDirectory(const QString &path);

signals:
    void showImage(const QModelIndex &index);
    void directoryLoaded(const QString &path);
    void rootPathChanged(const QString &path);

private slots:
    void modelReset();
    void modelArrange();

    bool emitShowImage(const QString &path);
    bool emitShowImage(const QModelIndex &index);

    void slotDirectoryLoaded(const QString &path);
    void slotRootPathChanged(const QString &path);

protected:
    virtual void geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry);

private:
    friend class DirectoryItem;
    friend class ThumbnailItem;

    FileSystemModel *m_model;
    FileSystemProxyModel *m_proxyModel;
    QRectF m_lastNewGeometry, m_lastOldGeometry;
    QColor m_folderNameFontColor, m_folderDetailsFontColor;
    QFont m_folderNameFont, m_folderDetailsFont;
    int m_imagesPerRow;
};

#endif // FILESYSTEMVIEW_H
