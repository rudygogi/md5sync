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
    QTimer::singleShot(0, this, [this](){ computeMd5();});
}

void Md5Processor::computeMd5(const QStringList &fileInfoList)
{
    m_shouldBreak = true;
    m_fileInfoList = fileInfoList;
    QTimer::singleShot(0, this, [this](){ computeMd5();});
}

void Md5Processor::computeMd5()
{
    m_shouldBreak = false;

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
                if (m_shouldBreak)
                {
                    qWarning() << "break";
                    return;
                }
            }
        }
        emit md5Computed(fileInfo, md5Hash.result());
    }
}

