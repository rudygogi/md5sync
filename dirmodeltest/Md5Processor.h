#pragma once

#include <QObject>

class Md5Processor : public QObject
{
    Q_OBJECT
public:
    Md5Processor();

public slots:
    void computeMd5(
            const QStringList& fileInfoList,
            int accuracyPercent = 10);

signals:
    void md5Computed(
            const QString& fileInfo,
            const QByteArray& md5);

private slots:
    void computeMd5();

private:
    bool m_shouldBreak{false};
    QStringList m_fileInfoList;
    int m_accuracyPercent;
};
