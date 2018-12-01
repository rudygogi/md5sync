#pragma once

#include <QFileSystemModel>

class Md5Model : public QFileSystemModel
{
    Q_OBJECT
public:
    Md5Model(QObject* parent);
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    QStringList getMissingMd5FileInfoList(const QModelIndex& parent) const;
    void setMd5(const QModelIndex& index, const QByteArray& md5);
private:
    QHash<QPersistentModelIndex, QByteArray> m_md5Hash;
};

