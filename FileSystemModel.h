#ifndef FILESYSTEMMODEL_H
#define FILESYSTEMMODEL_H

#include <QObject>
#include <QFileSystemModel>
#include <QPersistentModelIndex>
#include <QSet>

class QItemSelectionModel;

class FileSystemModel : public QFileSystemModel
{
    Q_OBJECT
    Q_ENUMS(Roles)
public:
    explicit FileSystemModel(QObject *parent = 0);
    virtual ~FileSystemModel();

    enum Roles {
        FileNameRole = Qt::UserRole + 100,
        FilePathRole,
        FileIconRole,
        FileSizeRole,
        FileTypeRole,
        FileIsDirRole,
        FileIsSelectedRole,
        FileIsCurrentRole,
        FileModelIndex
    };

public slots:
    bool fileExists(const QString &path) const;
    bool dirExists(const QString &dir) const;
    QString pathFromDirFile(const QString &directory, const QString &file) const;
    QString filenameFromPath(const QString &path) const;
    QString directoryFromPath(const QString &path) const;

    QString homePath() const;
    QString currentPath() const;

    int role(const QByteArray &roleName) const;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    QString directory() const;
    QModelIndex directoryIndex(const QString &path = QString()) const;

    QModelIndex setDirectory(const QString &directory);

    void setSelected(const QModelIndex &index, bool selected);
    QVariantList selected() const;

    //QObject* copySelectedJob(const QString &targetDirectory);
    //QObject* moveSelectedJob(const QString &targetDirectory);
    //QObject* mkdirJob(const QString &directory);
    //QObject* deleteSelectedJob();

private slots:
    void slotRootPathChanged(const QString &newPath);
    void slotDirectoryLoaded(const QString &path);
    void slotFileRenamed(const QString &path, const QString &oldName, const QString &newName);

private:
    QPersistentModelIndex m_rootIndex;
    QItemSelectionModel *m_selectionModel;
};

Q_DECLARE_METATYPE( QModelIndex )

#endif // FILESYSTEMMODEL_H
