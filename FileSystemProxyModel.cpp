#include "FileSystemProxyModel.h"
#include "FileSystemModel.h"

FileSystemProxyModel::FileSystemProxyModel(FileSystemModel *model)
    : QSortFilterProxyModel(model)
    , m_model(model)
{
    setFilterCaseSensitivity(Qt::CaseInsensitive);
    setFilterKeyColumn(0);
    setSortCaseSensitivity(Qt::CaseInsensitive);
    setDynamicSortFilter(true);
    sort(0, Qt::AscendingOrder);
    setSourceModel(m_model);
}

FileSystemProxyModel::~FileSystemProxyModel()
{
}

bool FileSystemProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    bool leftIsDir = m_model->data(left, FileSystemModel::FileIsDirRole).toBool();
    bool rightIsDir = m_model->data(right, FileSystemModel::FileIsDirRole).toBool();
    if (leftIsDir != rightIsDir)
        return leftIsDir;
    QString leftName = m_model->data(left, FileSystemModel::FileNameRole).toString().toLower();
    QString rightName = m_model->data(right, FileSystemModel::FileNameRole).toString().toLower();
    return leftName < rightName;
}
