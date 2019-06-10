#pragma once

#include <QObject>

class Md5Processor : public QObject
{
    Q_OBJECT
public:
    Md5Processor();

    void setPreferences(int chunkSize, int chunkStep);

public slots:
    void computeMd5(
            const QStringList& fileInfoList);

signals:
    void md5Computed(
            const QString& fileInfo,
            const QByteArray& md5);

private slots:
    void computeMd5Priv(const QStringList& fileInfoList);
    void computeMd5Priv();

private:
    bool m_shouldBreak{false};
    QStringList m_fileInfoList;
    int m_chunkSize;
    int m_chunkStep;
};
