#include "FileSystemView.h"
#include "FileSystemModel.h"
#include "FileSystemProxyModel.h"
#include "DirectoryItem.h"
#include "ImageItem.h"

#include <QGraphicsItemGroup>
#include <QGraphicsTextItem>
#include <QFont>
#include <QImageReader>
#include <QApplication>

class ThumbnailItem : public ImageItem
{
public:
    explicit ThumbnailItem(FileSystemView *view, const QModelIndex &index, qreal size)
        : ImageItem(view)
    {
        setTransformationMode(Qt::FastTransformation);
        setFlag(QGraphicsItem::ItemIsSelectable, true);
        //setFlag(QGraphicsItem::ItemIsFocusable, true);

        m_path = view->m_model->data(index, FileSystemModel::FilePathRole).toString();
        loadImage(m_path, size);
    }

protected:
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
    {
        QGraphicsPixmapItem::mouseReleaseEvent(event);
        static_cast<FileSystemView*>(parentItem())->emitShowImage(m_path);
    }

private:
    QString m_path;
};

FileSystemView::FileSystemView(QDeclarativeItem *parent)
    : QDeclarativeItem(parent)
    , m_model(new FileSystemModel(this))
    , m_proxyModel(new FileSystemProxyModel(m_model))
    , m_fontPixelSize(-1)
    , m_imagesPerRow(3)
{
    QStringList filters;
    Q_FOREACH(const QByteArray &format, QImageReader::supportedImageFormats())
        filters << QString("*.%1").arg(QString::fromUtf8(format));
    m_model->setNameFilters(filters);
    m_model->setNameFilterDisables(false);
    //m_model->setFilter(QDir::Files | QDir::NoDotAndDotDot);

    connect(m_model, SIGNAL(directoryLoaded(QString)), this, SLOT(modelReset()));
    connect(m_model, SIGNAL(rootPathChanged(QString)), this, SLOT(modelReset()));

    m_model->setDirectory(m_model->homePath());
}

QColor FileSystemView::fontColor() const
{
    if (m_fontColor.isValid())
        return m_fontColor;
    return QApplication::palette().color(QPalette::WindowText);
}

void FileSystemView::setFontColor(const QColor &color)
{
    m_fontColor = color;
}

int FileSystemView::fontPixelSize() const
{
    if (m_fontPixelSize >= 0)
        return m_fontPixelSize;
    return QApplication::font().pixelSize();
}

void FileSystemView::setFontPixelSize(int size)
{
    m_fontPixelSize = size;
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
    return ImageItem::threadPool()->maxThreadCount();
}

void FileSystemView::setThumbnailThreadCount(int count)
{
    ImageItem::threadPool()->setMaxThreadCount(count);
}

bool FileSystemView::inPortrait() const
{
    return height() > width();
}

void FileSystemView::modelReset()
{
    qDeleteAll(childItems());
    childItems().clear();
    QModelIndex directoryIndex = m_proxyModel->mapFromSource(m_model->directoryIndex());
    if (!directoryIndex.isValid())
        return;

    qreal x = 0.0;
    qreal y = 0.0;
    qreal w = width();
    qreal tw = w / m_imagesPerRow;

    QModelIndex parentDirectoryIndex = m_proxyModel->parent(directoryIndex);
    if (parentDirectoryIndex.isValid()) {
        QModelIndex index = m_proxyModel->mapToSource(parentDirectoryIndex);
        Q_ASSERT(index.isValid());
        DirectoryItem *item = new DirectoryItem(this, index, QLatin1String(".."));
        y += item->boundingRect().height();
    }

    int count = m_proxyModel->rowCount(directoryIndex);
    int row = 0;
    int column = 0;

    qDebug()<<"FileSystemView::modelReset count="<<count;
    for(int i = 0; i < count; ++i) {
        QModelIndex index = m_proxyModel->mapToSource(m_proxyModel->index(i, 0, directoryIndex));
        Q_ASSERT(index.isValid());

        bool isDir = m_model->data(index, FileSystemModel::FileIsDirRole).toBool();
        //qDebug()<<"FileSystemView::modelReset i="<<i<<"path="<<path;

        if (isDir) {
            QString name = m_model->data(index, FileSystemModel::FileNameRole).toString();
            DirectoryItem *item = new DirectoryItem(this, index, name);
            item->setPos(x, y);
            y += item->boundingRect().height();
            x = 0.0;
        } else {
            ThumbnailItem *item = new ThumbnailItem(this, index, tw);
            item->setPos(x, y);
            if (column == m_imagesPerRow - 1) {
                x = 0.0;
                y += tw;
                column = 0;
                ++row;
            } else {
                x += tw;
                ++column;
            }
        }
    }

    setImplicitHeight(y + (column == 0 ? 0 : tw));
}

void FileSystemView::modelArrange()
{
    qDebug()<<"FileSystemView::modelArrange imagesPerRow="<<m_imagesPerRow<<"inPortrait="<<inPortrait()<<"width="<<width()<<"height="<<height()<<"boundingRect="<<boundingRect();

    qreal x = 0.0;
    qreal y = 0.0;
    qreal w = width();
    qreal tw = w / m_imagesPerRow;
    int row = 0;
    int column = 0;
    Q_FOREACH(QGraphicsItem *item, childItems()) {
        if (!dynamic_cast<ThumbnailItem*>(item)) {
            y += item->boundingRect().height();
            continue;
        }
        item->setPos(x, y);
        //qreal scale = thumbnailItem->boundingRect().width() / tw;
        //thumbnailItem->setScale(scale);
        if (column == m_imagesPerRow - 1) {
            x = 0.0;
            y += tw;
            column = 0;
            ++row;
        } else {
            x += tw;
            ++column;
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

void FileSystemView::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    qDebug() << "FileSystemView::geometryChanged newGeometry=" << newGeometry << "oldGeometry=" << oldGeometry;
    QDeclarativeItem::geometryChanged(newGeometry, oldGeometry);
    modelArrange();
}

