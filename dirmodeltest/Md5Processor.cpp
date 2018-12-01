#include "Md5Processor.h"

#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDebug>
#include <QFile>
#include <QTimer>

Md5Processor::Md5Processor()
{

}

void Md5Processor::computeMd5(const QStringList &fileInfoList, int accuracyPercent)
{
    m_shouldBreak = true;
    m_fileInfoList = fileInfoList;
    m_accuracyPercent = accuracyPercent;
    QTimer::singleShot(0, this, [this](){ computeMd5();});
}

void Md5Processor::computeMd5()
{
    m_shouldBreak = false;
    int readByteCount = 100;
    if (m_accuracyPercent < 1)
    {
        readByteCount = 1;
    }
    else if (m_accuracyPercent < 100)
    {
        readByteCount = m_accuracyPercent;
    }
    int skipByteCount = (100 - readByteCount) * 100;
    readByteCount = readByteCount * 100;

    for(QString fileInfo : m_fileInfoList)
    {
        qInfo() << fileInfo;
        QFile file(fileInfo);
        if (!file.open(QIODevice::ReadOnly))
        {
            emit md5Computed(fileInfo, QByteArray());
            continue;
        }
        QCryptographicHash md5Hash(QCryptographicHash::Md5);
        while(!file.atEnd())
        {
            int pos = file.pos();
            md5Hash.addData(file.read(readByteCount));
            file.seek(pos + readByteCount + skipByteCount);
            QCoreApplication::processEvents();
            if (m_shouldBreak)
            {
                qWarning() << "break";
                return;
            }
        }
        emit md5Computed(fileInfo, md5Hash.result());
    }
}

