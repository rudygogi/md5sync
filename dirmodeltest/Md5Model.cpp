#include "Md5Model.h"

Md5Model::Md5Model(QObject* parent) :
    QFileSystemModel(parent)
{
}

int Md5Model::columnCount(const QModelIndex &parent) const
{
    int count = QFileSystemModel::columnCount(parent);
    if (count == 0)
    {
        return 0;
    }
    else
    {
        return count + 1;
    }
}

QVariant Md5Model::data(const QModelIndex &index, int role) const
{
    int md5Column = QFileSystemModel::columnCount();
    if (index.column() < md5Column)
    {
        return QFileSystemModel::data(index, role);
    }
    else
    {
        if (role == Qt::DisplayRole)
        {
            if (isDir(index))
            {
                return "-";
            }
            else
            {
                if (!m_md5Hash.contains(index))
                {
                    return "...";
                }
                else
                {
                    QByteArray md5 = m_md5Hash[index];
                    if (md5.isEmpty())
                    {
                        return "not readable";
                    }
                    else
                    {
                        return md5.toHex();
                    }
                }
            }
        }
        else
        {
            return QVariant();
        }
    }
}

QVariant Md5Model::headerData(int section, Qt::Orientation orientation, int role) const
{
    int count = QFileSystemModel::columnCount();
    if (section < count || orientation != Qt::Horizontal || role != Qt::DisplayRole)
    {
        return QFileSystemModel::headerData(section, orientation, role);
    }
    return "MD5";
}

QStringList Md5Model::getMissingMd5FileInfoList(const QModelIndex& parent) const
{
    int md5Column = QFileSystemModel::columnCount();
    QStringList fileInfoList;
    for(int row = 0; row < rowCount(parent); ++row)
    {
        QModelIndex md5Index = index(row, md5Column, parent);
        if (!isDir(md5Index) && !m_md5Hash.contains(md5Index))
        {
            fileInfoList.append(filePath(md5Index));
        }
    }
    return fileInfoList;
}

void Md5Model::setMd5(const QModelIndex& index, const QByteArray& md5)
{
    int md5Column = QFileSystemModel::columnCount();
    QModelIndex md5Index = index.sibling(index.row(), md5Column);
    m_md5Hash[md5Index] = md5;
    emit dataChanged(md5Index, md5Index, {Qt::DisplayRole});
}
