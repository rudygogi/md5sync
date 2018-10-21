#ifndef MD5SYNCWIDGET_H
#define MD5SYNCWIDGET_H

#include <QWidget>
class QLineEdit;
class QAbstractItemView;
#include <functional>

namespace Ui {
class Md5SyncWidget;
}

class Md5SyncWidget : public QWidget
{
    Q_OBJECT

public:
    explicit Md5SyncWidget(QWidget *parent = 0);
    ~Md5SyncWidget();

private:
    void showOpenDirDialog(QLineEdit* dirLineEdit);

    void restoreSettings(
            const QString& settingsName,
            std::function<void(const QVariant& value)> setFunction);

    void updatePath(QLineEdit* pathLineEdit, QAbstractItemView* itemView);

private:
    Ui::Md5SyncWidget *ui;

    const QString GEOMETRY_SETTING = "geometry";
    const QString LEFT_PATH_SETTING = "leftpath";
    const QString RIGHT_PATH_SETTING = "rightpath";
};

#endif // MD5SYNCWIDGET_H
