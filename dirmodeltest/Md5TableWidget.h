#pragma once

class Md5Model;
class Md5Processor;

#include <QWidget>
#include <QScrollBar>

namespace Ui {
class Md5TableWidget;
}

class Md5TableWidget : public QWidget
{
    Q_OBJECT

public:
    explicit Md5TableWidget(QWidget *parent = nullptr);
    ~Md5TableWidget();

    enum ScrollBarPosition
    {
        POSITION_LEFT,
        POSITION_RIGHT
    };
    void setScrollBarPosition(ScrollBarPosition pos);

    int getTableTopPos() const;
    QHash<QByteArray, QSet<int> > getMd5PositionHash() const;
    QSet<QByteArray> getSelectedMd5Set() const;

signals:
    void dataChanged();

private:
    void onDoubleClicked(const QModelIndex &index);
    void setMd5(const QString& fileInfo, const QByteArray& md5);
    void requestMd5();
    void cdUp();
    void cd();
    void updatePathLine();

    void resizeSections();

private:
    Ui::Md5TableWidget *m_ui;
    Md5Model* m_model { nullptr };
    QThread* m_md5Thread { nullptr };
    Md5Processor* m_md5Worker { nullptr };
    QScrollBar* m_scrollBar;
};

