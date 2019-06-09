#ifndef SETTINGS_H
#define SETTINGS_H

#include <QSettings>

class Settings : public QSettings
{
public:
    Settings();

    void setChunkSize(int size);
    int getChunkSize() const;

    void setChunkStep(int step);
    int getChunkStep() const;

    void setLeftPath(const QString& path);
    QString getLeftPath() const;

    void setRightPath(const QString& path);
    QString getRightPath() const;
};

#endif // SETTINGS_H
