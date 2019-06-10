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
    m_shouldBreak = true;
    QTimer::singleShot(0, this, [=]() { readIcons(fileNameList); });
}

void IconProvider::readIcons(const QStringList& fileNameList)
{
    m_shouldBreak = false;
    QFileIconProvider iconProv;
    for(auto fileName : fileNameList)
    {
        if (m_shouldBreak)
        {
            return;
        }

        QIcon icon;
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

DummyFileIconProvider::DummyFileIconProvider() :
    QFileIconProvider()
{

}

QIcon DummyFileIconProvider::icon(const QFileInfo &info) const
{
    if (info.isDir())
    {
        return QFileIconProvider::icon(Folder);
    }
    else
    {
        return QFileIconProvider::icon(File);
    }
}
