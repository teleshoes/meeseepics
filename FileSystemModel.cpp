#include "FileSystemModel.h"

#include <QMetaObject>
#include <QMetaEnum>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QItemSelectionModel>
#include <QDateTime>
#include <QDebug>

inline QString formatSize(qint64 rsz)
{
    double sz = (double)rsz;
    if ( sz >= (100.0 * 1024.0 * 1024.0 * 1024.0) ) {
        sz = sz / (1024.0 * 1024.0 * 1024.0);
        return QObject::tr("%1G").arg(sz, 0, 'f', 1);
    } else if ( sz >= (10.0 * 1024.0 * 1024.0 * 1024.0) ) {
        sz = sz / (1024.0 * 1024.0 * 1024.0);
        return QObject::tr("%1G").arg(sz, 0, 'f', 2);
    } else if ( sz >= (1024.0 * 1024.0 * 1024.0) ) {
        sz = sz / (1024.0 * 1024.0 * 1024.0);
        return QObject::tr("%1G").arg(sz, 0, 'f', 3);
    } else if ( sz >= (1024.0 * 1024.0) ) {
        sz = sz / (1024.0 * 1024.0);
        return QObject::tr("%1M").arg(sz, 0, 'f', 1);
    } else if ( sz >= 1024.0) {
        sz = sz / 1024.0;
        return QObject::tr("%1K").arg(sz, 0, 'f', 1);
    }
    return QString::number(sz, 'f', 0);
}

FileSystemModel::FileSystemModel(QObject *parent)
    : QFileSystemModel(parent)
{
    setFilter(QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot);
    //setFilter(QDir::AllDirs | QDir::Files | QDir::Hidden | QDir::NoDotAndDotDot);

    QHash<int, QByteArray> roles;
    roles[FileNameRole] = "fileName";
    roles[FilePathRole] = "filePath";
    roles[FileIconRole] = "fileIcon";
    roles[FileSizeRole] = "fileSize";
    roles[FileTypeRole] = "fileType";
    roles[FileModifiedRole] = "fileModified";
    roles[FileIsDirRole] = "fileIsDirectory";
    roles[FileIsSelectedRole] = "fileIsSelected";
    roles[FileIsCurrentRole] = "fileIsCurrent";
    roles[FileModelIndex] = "fileModelIndex";
    setRoleNames(roles);

    m_selectionModel = new QItemSelectionModel(this);

    qRegisterMetaType<QModelIndex>("QModelIndex");
    //qRegisterMetaType<QModelIndexList>("QModelIndexList");
    //qRegisterMetaType<QPersistentModelIndex>("QPersistentModelIndex");

    connect(this, SIGNAL(rootPathChanged(QString)), this, SLOT(slotRootPathChanged(QString)));
    connect(this, SIGNAL(directoryLoaded(QString)), this, SLOT(slotDirectoryLoaded(QString)));
    connect(this, SIGNAL(fileRenamed(QString,QString,QString)), this, SLOT(slotFileRenamed(QString,QString,QString)));
}

FileSystemModel::~FileSystemModel()
{
}

bool FileSystemModel::fileExists(const QString &path) const
{
    QFileInfo fi(path);
    return fi.isFile() && fi.exists();
}

bool FileSystemModel::dirExists(const QString &dir) const
{
    QDir d(dir);
    return d.exists();
}

QString FileSystemModel::pathFromDirFile(const QString &directory, const QString &file) const
{
    QFileInfo fi(directory, file);
    return fi.filePath();
}

QString FileSystemModel::filenameFromPath(const QString &path) const
{
    QFileInfo fi(path);
    return fi.fileName();
}

QString FileSystemModel::directoryFromPath(const QString &path) const
{
    QFileInfo fi(path);
    QString s = fi.absolutePath();
    if (!s.isEmpty() && !s.endsWith(QDir::separator()))
        s += QDir::separator();
    return s;
}

QString FileSystemModel::homePath() const
{
    return QDir::homePath() + QDir::separator();
}

QString FileSystemModel::currentPath() const
{
    return QDir::currentPath() + QDir::separator();
}

int FileSystemModel::role(const QByteArray &roleName) const
{
    QMetaEnum e = metaObject()->enumerator(metaObject()->indexOfEnumerator("Roles"));
    Q_ASSERT(e.isValid());
    return e.keyToValue(roleName);
}

QVariant FileSystemModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        qDebug() << "FileSystemModel::data Invalid index";
        return QVariant();
    }
    switch (role) {
        case Qt::DisplayRole:
        case QFileSystemModel::FileNameRole:
        case FileNameRole:
            return QFileSystemModel::data(index, QFileSystemModel::FileNameRole);
        case QFileSystemModel::FileIconRole: // equals Qt::DecorationRole
        case FileIconRole:
            return QFileSystemModel::data(index, QFileSystemModel::FileIconRole);
        case FilePathRole:
        case QFileSystemModel::FilePathRole: {
            QString s = QFileSystemModel::data(index, QFileSystemModel::FilePathRole).toString();
            if (!s.isEmpty() && isDir(index) && !s.endsWith(QDir::separator()))
                s += QDir::separator();
            return s;
        }
        case QFileSystemModel::FilePermissions:
            return QFileSystemModel::data(index, QFileSystemModel::FilePermissions);
        case FileSizeRole:
            return isDir(index) ? QString() : formatSize(size(index));
        case FileTypeRole:
            return type(index);
        case FileModifiedRole:
            return lastModified(index);
        case FileIsDirRole:
            return isDir(index);
        case FileIsSelectedRole:
            return m_selectionModel->isSelected(index);
        case FileIsCurrentRole:
            return m_selectionModel->currentIndex() == index;
        case FileModelIndex:
            return QVariant::fromValue<QModelIndex>(index);
        default:
            break;
    }
    qDebug() << "FileSystemModel::data Invalid role=" << role;
    return QVariant();
}

QString FileSystemModel::directory() const
{
    QString s = rootPath();
    if (!s.isEmpty() && !s.endsWith(QDir::separator()))
        s += QDir::separator();
    return s;
}

QModelIndex FileSystemModel::directoryIndex(const QString &path) const
{
    QModelIndex idx = path.isEmpty() ? QModelIndex(m_rootIndex) : index(path);
    if (!idx.isValid()) {
        qWarning() << "FileSystemModel::directoryIndex Invalid source QModelIndex for directory=" << (path.isEmpty() ? rootPath() : path);
        return QModelIndex();
    }
    return idx;
}

QModelIndex FileSystemModel::setDirectory(const QString &directory)
{
    m_rootIndex = setRootPath(directory);
    qDebug() << "FileSystemModel::setDirectory directory=" << directory << "isValid=" << m_rootIndex.isValid();
    m_selectionModel->clearSelection();
    return m_rootIndex;
}

QVariantList FileSystemModel::selected() const
{
    QVariantList list;
    Q_FOREACH(const QModelIndex &index, m_selectionModel->selectedIndexes())
        list.append(QVariant::fromValue<QModelIndex>(index));
    qDebug() << "FileSystemModel::selected list=" << list;
    return list;
}

void FileSystemModel::setSelected(const QModelIndex &index, bool selected)
{
    if (!index.isValid()) {
        qDebug() << "FileSystemModel::setSelected Invalid index=" << index << "selected=" << selected;
        return;
    }
    QItemSelectionModel::SelectionFlags flags(selected ? QItemSelectionModel::Select : QItemSelectionModel::Deselect);
    m_selectionModel->select(index, flags);
    emit dataChanged(index, index);
}

void FileSystemModel::slotRootPathChanged(const QString &newPath)
{
    QString currentRootPath = rootPath();
    qDebug() << "FileSystemModel::slotRootPathChanged newPath=" << newPath << "currentRootPath=" << currentRootPath;
    if (currentRootPath != newPath)
        setDirectory(newPath);
}

void FileSystemModel::slotDirectoryLoaded(const QString &path)
{
    QString currentRootPath = rootPath();
    qDebug() << "FileSystemModel::slotDirectoryLoaded path=" << path << "currentRootPath=" << currentRootPath;
    if (currentRootPath != path)
        setDirectory(path);
}

void FileSystemModel::slotFileRenamed(const QString &path, const QString &oldName, const QString &newName)
{
    qDebug() << "FileSystemModel::slotFileRenamed path=" << path << "oldName=" << oldName << "newName=" << newName;
}
