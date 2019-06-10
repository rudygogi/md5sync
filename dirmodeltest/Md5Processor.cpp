#include "Md5Processor.h"

#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDebug>
#include <QFile>
#include <QTimer>

Md5Processor::Md5Processor()
{

}

void Md5Processor::setPreferences(int chunkSize, int chunkStep)
{
    m_chunkSize = chunkSize;
    m_chunkStep = chunkStep;
    m_shouldBreak = true;
    QTimer::singleShot(0, this, [this](){ computeMd5Priv();});
}

void Md5Processor::computeMd5(const QStringList &fileInfoList)
{
    m_shouldBreak = true;    
    QTimer::singleShot(0, this, [=](){ computeMd5Priv(fileInfoList);});
}

void Md5Processor::computeMd5Priv(const QStringList& fileInfoList)
{
    m_fileInfoList = fileInfoList;
    computeMd5Priv();
}
void Md5Processor::computeMd5Priv()
{
    m_shouldBreak = false;
    qInfo() << "Computing MD5 started";
    for(QString fileInfo : m_fileInfoList)
    {
        if (m_shouldBreak)
        {
            qWarning() << "break";
            return;
        }

        qInfo() << fileInfo;
        QFile file(fileInfo);
        if (!file.open(QIODevice::ReadOnly))
        {
            emit md5Computed(fileInfo, QByteArray());
            continue;
        }
        QCryptographicHash md5Hash(QCryptographicHash::Md5);
        if (m_chunkSize == 0)
        {
            md5Hash.addData(file.readAll());
        }
        else
        {
            while(!file.atEnd())
            {
                int pos = file.pos();
                md5Hash.addData(file.read(m_chunkSize));
                if (m_chunkStep == 0)
                {
                    break;
                }
                file.seek(pos + m_chunkStep);
            }
        }
        emit md5Computed(fileInfo, md5Hash.result());
    }
    qInfo() << "Computing MD5 finished";
}

