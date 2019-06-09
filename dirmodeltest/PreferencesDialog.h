#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include <QDialog>

namespace Ui {
class PreferencesDialog;
}

class PreferencesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PreferencesDialog(int chunkSize, int chunkStep, QWidget *parent = 0);
    ~PreferencesDialog();

    int getChunkSize() const;
    int getChunkStep() const;

private:
    Ui::PreferencesDialog *ui;
};

#endif // PREFERENCESDIALOG_H
