#include "Md5Model.h"

Md5Model::Md5Model(QObject* parent) :
    QFileSystemModel(parent),
    m_md5Column(QFileSystemModel::columnCount())
{
}

int Md5Model::columnCount(const QModelIndex &) const
{
    return m_md5Column + 1;
}

QVariant Md5Model::data(const QModelIndex &index, int role) const
{
    if (index.column() < m_md5Column)
    {
        return QFileSystemModel::data(index, role);
    }
    if (role != Qt::DisplayRole)
    {
        return QVariant();
    }
    if (isDir(index))
    {
        return "-";
    }
    if (!m_md5Hash.contains(index))
    {
        return "...";
    }
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

QVariant Md5Model::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (section < m_md5Column || orientation != Qt::Horizontal || role != Qt::DisplayRole)
    {
        return QFileSystemModel::headerData(section, orientation, role);
    }
    return "MD5";
}

QStringList Md5Model::getMissingMd5FileInfoList(const QModelIndex& parent) const
{
    QStringList fileInfoList;
    for(int row = 0; row < rowCount(parent); ++row)
    {
        QModelIndex md5Index = index(row, m_md5Column, parent);
        if (!isDir(md5Index) && !m_md5Hash.contains(md5Index))
        {
            fileInfoList.append(filePath(md5Index));
        }
    }
    return fileInfoList;
}

void Md5Model::setMd5(const QModelIndex& index, const QByteArray& md5)
{
    QModelIndex md5Index = index.sibling(index.row(), m_md5Column);
    m_md5Hash[md5Index] = md5;
    emit dataChanged(md5Index, md5Index, {Qt::DisplayRole});
}

QByteArray Md5Model::getMd5(const QModelIndex &index) const
{
    QModelIndex md5Index = index.sibling(index.row(), m_md5Column);
    return m_md5Hash.value(md5Index);
}

void Md5Model::resetMd5Hash()
{
    m_md5Hash.clear();
}
