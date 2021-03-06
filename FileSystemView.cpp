#include "FileSystemView.h"
#include "FileSystemModel.h"
#include "FileSystemProxyModel.h"
#include "DirectoryItem.h"
#include "ThumbnailItem.h"

#include <QGraphicsItemGroup>
#include <QGraphicsTextItem>
#include <QFont>
#include <QImageReader>
#include <QApplication>
#include <QTimer>

FileSystemView::FileSystemView(QDeclarativeItem *parent)
    : QDeclarativeItem(parent)
    , m_model(new FileSystemModel(this))
    , m_proxyModel(new FileSystemProxyModel(m_model))
    , m_imagesPerRow(3)
    , m_detailedFileList(false)
{
    QStringList filters;
    Q_FOREACH(const QByteArray &format, QImageReader::supportedImageFormats())
        filters << QString("*.%1").arg(QString::fromUtf8(format));
    m_model->setNameFilters(filters);
    m_model->setNameFilterDisables(false);
    //m_model->setFilter(QDir::Files | QDir::NoDotAndDotDot);

    connect(m_model, SIGNAL(directoryLoaded(QString)), this, SLOT(slotDirectoryLoaded(QString)));
    connect(m_model, SIGNAL(rootPathChanged(QString)), this, SLOT(slotRootPathChanged(QString)));
}

void FileSystemView::componentComplete()
{
    QDeclarativeItem::componentComplete();

    QString dir = m_model->homePath();
    QString file;
    if (QApplication::arguments().count() >= 2) {
        for(int i = 1; i < QApplication::arguments().count(); ++i) {
            QString arg = QApplication::arguments().at(i);
            QFileInfo fi(arg);
            QDir d(arg);
            if (d.exists()) {
                dir = "" + d.absolutePath();
                break;
            }else if (fi.exists()) {
                dir = "" + fi.absolutePath();
                file = "" + fi.absoluteFilePath();
                break;
            }
        }
    }

    m_model->setDirectory(dir);
    if (!file.isEmpty()) {
        savedImageFile = file;
        QTimer::singleShot(100, this, SLOT(revealSavedImage()));
    }
}

QColor FileSystemView::folderNameFontColor() const
{
    if (m_folderNameFontColor.isValid())
        return m_folderNameFontColor;
    return QApplication::palette().color(QPalette::WindowText);
}

void FileSystemView::setFolderNameFontColor(const QColor &color)
{
    qDebug()<<"FileSystemView::setFolderNameFontColor color="<<color<<"isValid="<<color.isValid();
    m_folderNameFontColor = color;
    modelArrange();
}

QFont FileSystemView::folderNameFont() const
{
    return m_folderNameFont;
}

void FileSystemView::setFolderNameFont(const QFont &font)
{
    m_folderNameFont = font;
    //modelArrange();
}

QColor FileSystemView::folderDetailsFontColor() const
{
    if (m_folderDetailsFontColor.isValid())
        return m_folderDetailsFontColor;
    return QApplication::palette().color(QPalette::WindowText);
}

void FileSystemView::setFolderDetailsFontColor(const QColor &color)
{
    m_folderDetailsFontColor = color;
    modelArrange();
}

QFont FileSystemView::folderDetailsFont() const
{
    return m_folderDetailsFont;
}

void FileSystemView::setFolderDetailsFont(const QFont &font)
{
    m_folderDetailsFont = font;
    //modelArrange();
}

int FileSystemView::imagesPerRow() const
{
    return m_imagesPerRow;
}

void FileSystemView::setImagesPerRow(int number)
{
    number = qMax(1, number);
    if (m_imagesPerRow == number)
        return;
    m_imagesPerRow = number;
    modelArrange();
}

int FileSystemView::thumbnailThreadCount() const
{
    return ImageItem::thumbnailThreadPool()->maxThreadCount();
}

void FileSystemView::setThumbnailThreadCount(int count)
{
    ImageItem::thumbnailThreadPool()->setMaxThreadCount(count);
}

int FileSystemView::imageThreadCount() const
{
    return ImageItem::imageThreadPool()->maxThreadCount();
}

void FileSystemView::setImageThreadCount(int count)
{
    ImageItem::imageThreadPool()->setMaxThreadCount(count);
}

bool FileSystemView::detailedFileList() const
{
    return m_detailedFileList;
}

void FileSystemView::setDetailedFileList(bool enabled)
{
    m_detailedFileList = enabled;
    modelArrange();
}

void FileSystemView::setDirectory(const QString &path)
{
    m_model->setDirectory(path);
}

void FileSystemView::revealSavedImage()
{
    if (!savedImageFile.isEmpty()) {
        emitShowImage(savedImageFile);
    }
}

void FileSystemView::modelReset()
{
    qDeleteAll(childItems());

    QModelIndex directoryIndex = m_proxyModel->mapFromSource(m_model->directoryIndex());
    if (!directoryIndex.isValid()) {
        return;
    }

    if (directoryIndex != m_proxyModel->mapFromSource(m_model->index(m_model->directory()))) {
        return;
    }

    QGraphicsItem *mainItem = new QGraphicsRectItem(this);

    qreal x = 0.0;
    qreal y = 0.0;
    qreal w = width();
    qreal tw = w / m_imagesPerRow;

    int count = m_proxyModel->rowCount(directoryIndex);
    int row = 0;
    int column = 0;

    QModelIndex parentDirectoryIndex = m_proxyModel->parent(directoryIndex);
    if (parentDirectoryIndex.isValid()) {
        QModelIndex index = m_proxyModel->mapToSource(parentDirectoryIndex);
        Q_ASSERT(index.isValid());
        DirectoryItem *item = new DirectoryItem(this, mainItem, index, QLatin1String(".."));
        y += item->boundingRect().height();
    }

    for(int i = 0; i < count; ++i) {
        QModelIndex index = m_proxyModel->mapToSource(m_proxyModel->index(i, 0, directoryIndex));
        Q_ASSERT(index.isValid());

        bool isDir = m_model->data(index, FileSystemModel::FileIsDirRole).toBool();
        //qDebug()<<"FileSystemView::modelReset i="<<i<<"path="<<path;

        if (isDir) {
            QString name = m_model->data(index, FileSystemModel::FileNameRole).toString();
            DirectoryItem *item = new DirectoryItem(this, mainItem, index, name);
            item->setPos(x, y);
            y += item->boundingRect().height();
            x = 0.0;
        } else {
            ThumbnailItem *item = new ThumbnailItem(this, mainItem, index, tw);
            item->setPos(x, y);
            item->setDetailedFileList(m_detailedFileList);
            item->nextThumbnailPosition(column, row, x, y, tw);
        }
    }

    setImplicitHeight(y + (column == 0 ? 0 : tw));
}

void FileSystemView::modelArrange()
{
    //qDebug()<<"FileSystemView::modelArrange imagesPerRow="<<m_imagesPerRow<<"inPortrait="<<inPortrait()<<"width="<<width()<<"height="<<height()<<"boundingRect="<<boundingRect();

    qreal x = 0.0;
    qreal y = 0.0;
    qreal w = width();
    qreal tw = w / m_imagesPerRow;
    int row = 0;
    int column = 0;
    Q_FOREACH(QGraphicsItem *mainItem, childItems()) {
        Q_FOREACH(QGraphicsItem *item, mainItem->childItems()) {
            if (ThumbnailItem *thumbnailItem = dynamic_cast<ThumbnailItem*>(item)) {
                thumbnailItem->setPos(x, y);
                thumbnailItem->setDetailedFileList(m_detailedFileList);
                thumbnailItem->nextThumbnailPosition(column, row, x, y, tw);
            } else {
                if (DirectoryItem *directoryItem = dynamic_cast<DirectoryItem*>(item))
                    directoryItem->updateView();
                y += item->boundingRect().height();
            }
        }
    }

    setImplicitHeight(y + (column == 0 ? 0 : tw));
}

bool FileSystemView::emitShowImage(const QString &path)
{
    QModelIndex index = m_model->index(path);
    if (!index.isValid()) {
        qWarning() << "FileSystemView::emitShowImage No such path=" << path;
        return false;
    }
    return emitShowImage(index);
}

bool FileSystemView::emitShowImage(const QModelIndex &index)
{
    Q_ASSERT(index.isValid());
    QModelIndex idx;
    if (index.model() == m_model) {
        idx = m_proxyModel->mapFromSource(index);
    } else if (index.model() == m_proxyModel) {
        idx = index;
    } else {
        qWarning() << "FileSystemView::emitShowImage Invalid modelIndex=" << index.model();
        return false;
    }
    Q_ASSERT(idx.isValid());
    emit showImage(idx);
    return true;
}

void FileSystemView::slotDirectoryLoaded(const QString &path)
{
    modelReset();
    emit directoryLoaded(path);
}

void FileSystemView::slotRootPathChanged(const QString &path)
{
    modelReset();
    emit rootPathChanged(path);
}

void FileSystemView::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    //qDebug() << "FileSystemView::geometryChanged newGeometry=" << newGeometry << "oldGeometry=" << oldGeometry;
    QDeclarativeItem::geometryChanged(newGeometry, oldGeometry);
    if (newGeometry == oldGeometry || (m_lastNewGeometry == newGeometry && m_lastOldGeometry == oldGeometry))
        return;
    m_lastNewGeometry = newGeometry;
    m_lastOldGeometry = oldGeometry;
    modelArrange();
}
