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
    void readIcons(const QStringList& fileNameList);
private:
    bool m_shouldBreak{false};
};

#include <QFileIconProvider>

class DummyFileIconProvider : public QFileIconProvider
{
public:
    DummyFileIconProvider();

    QIcon icon(const QFileInfo &info) const override;
};

#endif // ICONPROVIDER_H
