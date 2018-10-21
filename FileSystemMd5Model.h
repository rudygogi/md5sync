#ifndef FILESYSTEMMD5MODEL_H
#define FILESYSTEMMD5MODEL_H

#include <QFileSystemModel>

class FileSystemMd5Model : public QFileSystemModel
{
public:
    FileSystemMd5Model(QObject* parent = 0);

    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

private:
    QHash<QString, QString> m_filePathMd5Hash;
};

#endif // FILESYSTEMMD5MODEL_H
