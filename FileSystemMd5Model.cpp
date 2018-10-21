#include "FileSystemMd5Model.h"

FileSystemMd5Model::FileSystemMd5Model(QObject *parent) :
    QFileSystemModel(parent)
{

}

int FileSystemMd5Model::columnCount(const QModelIndex &parent) const
{
    return QFileSystemModel::columnCount(parent) + 1;
}

QVariant FileSystemMd5Model::data(const QModelIndex &index, int role) const
{
    if (index.column() < QFileSystemModel::columnCount(index.parent()))
    {
        return QFileSystemModel::data(index, role);
    }
    if (role != Qt::DisplayRole && role != Qt::EditRole)
    {
        return data(index.sibling(index.row(), index.column() - 1), role);
    }
    QString path = filePath(index);
    QString md5 = m_filePathMd5Hash.value(path);
    return md5;
}

QVariant FileSystemMd5Model::headerData(int section, Qt::Orientation orientation, int role) const
{
    if ((orientation != Qt::Horizontal) ||
            (section < QFileSystemModel::columnCount()))
    {
        return QFileSystemModel::headerData(section, orientation, role);
    }
    if (role != Qt::DisplayRole && role != Qt::EditRole)
    {
        return headerData(section - 1, orientation,role);
    }
    return "MD5";
}
