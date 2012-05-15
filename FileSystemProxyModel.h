#ifndef FILESYSTEMPROXYMODEL_H
#define FILESYSTEMPROXYMODEL_H

#include <QSortFilterProxyModel>

class FileSystemModel;

class FileSystemProxyModel : public QSortFilterProxyModel
{
public:
    explicit FileSystemProxyModel(FileSystemModel *model);
    virtual ~FileSystemProxyModel();

protected:
    virtual bool lessThan(const QModelIndex &left, const QModelIndex &right) const;

private:
    FileSystemModel *m_model;
};

#endif // FILESYSTEMPROXYMODEL_H
