#pragma once

class Md5Model;
class Md5Processor;
class IconProvider;

#include <QWidget>
#include <QScrollBar>
#include <QSet>

namespace Ui {
class Md5TableWidget;
}

class Md5TableWidget : public QWidget
{
    Q_OBJECT

public:
    explicit Md5TableWidget(QWidget *parent = nullptr);
    ~Md5TableWidget();

    void setRootPath(const QString& path);
    QString getRootPath() const;

    enum ScrollBarPosition
    {
        POSITION_LEFT,
        POSITION_RIGHT
    };
    void setScrollBarPosition(ScrollBarPosition pos);

    int getTableTopPos() const;
    int getTableHeight() const;

    void setPreferences(int chunkSize, int chunkStep);

    QHash<QByteArray, QSet<int> > getMd5PositionHash() const;
    QSet<QByteArray> getSelectedMd5Set() const;

    void setOtherTableData(
            const QString& dirPath,
            const QSet<QByteArray>& md5Set);
    QString getDirPath() const;
    QSet<QByteArray> getMd5Set() const;

signals:
    void dataChanged();

private:
    void onDoubleClicked(const QModelIndex &index);
    void setMd5(const QString& fileInfo, const QByteArray& md5);
    void requestMd5();
    void cdUp();
    void cd();
    void updatePathLine();

    void onDirectoryLoaded();

    void resizeSections();

    void showSelectedPreviews();
    void showIcon(const QString& fileName, const QIcon& icon);

    void selectAll();
    void selectNone();
    void selectDuplicates();
    void selectPresent();
    void selectMissing();

    void cleanFiles();
    void copyFiles();
    void moveFiles();

private:
    Ui::Md5TableWidget *m_ui;
    Md5Model* m_model { nullptr };

    QThread* m_md5Thread { nullptr };
    Md5Processor* m_md5Worker { nullptr };

    QThread* m_iconThread{nullptr};
    IconProvider* m_iconWorker{nullptr};

    QScrollBar* m_scrollBar;

    QString m_otherDirPath;
    QSet<QByteArray> m_otherMd5Set;
};

