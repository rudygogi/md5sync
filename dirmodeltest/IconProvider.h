#ifndef ICONPROVIDER_H
#define ICONPROVIDER_H

#include <QObject>
#include <QIcon>

class IconProvider : public QObject
{
    Q_OBJECT
public:
    IconProvider(QObject* parent = nullptr);

public:
    void provideIcons(const QStringList& fileNameList);

signals:
    void iconReady(const QString& fileName, const QIcon& icon);

private:
    void readIcons();
private:
    QStringList m_fileNameList;
};

#endif // ICONPROVIDER_H
