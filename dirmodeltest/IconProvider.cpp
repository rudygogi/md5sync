#include "IconProvider.h"

#include <QTimer>
#include <QImageReader>
#include <QFileIconProvider>

IconProvider::IconProvider(QObject *parent) :
    QObject(parent)
{

}

void IconProvider::provideIcons(const QStringList &fileNameList)
{
    m_fileNameList = fileNameList;
    QTimer::singleShot(0, this, [this]() { readIcons(); });
}

void IconProvider::readIcons()
{
    QFileIconProvider iconProv;
    while(!m_fileNameList.isEmpty())
    {
        QIcon icon;
        QString fileName = m_fileNameList.takeFirst();
        QImageReader imageReader(fileName);
        imageReader.setAutoTransform(true);
        QImage image = imageReader.read();
        if (!image.isNull())
        {
            QPixmap pixmap;
            pixmap.convertFromImage(image);
            icon = QIcon(pixmap.scaled(QSize(128, 128), Qt::KeepAspectRatio));
        }
        else
        {
            icon = iconProv.icon(fileName);
        }
        emit iconReady(fileName, icon);
    }
}
