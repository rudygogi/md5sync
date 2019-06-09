#include "Settings.h"

Settings::Settings()
{

}

void Settings::setChunkSize(int size)
{
    setValue("chunksize", size);
}

int Settings::getChunkSize() const
{
    return value("chunksize").toInt();
}

void Settings::setChunkStep(int step)
{
    setValue("chunkstep", step);
}

int Settings::getChunkStep() const
{
    return value("chunkstep").toInt();
}

void Settings::setLeftPath(const QString &path)
{
    setValue("leftpath", path);
}

QString Settings::getLeftPath() const
{
    return value("leftpath").toString();
}

void Settings::setRightPath(const QString &path)
{
    setValue("rightpath", path);
}

QString Settings::getRightPath() const
{
    return value("rightpath").toString();
}
