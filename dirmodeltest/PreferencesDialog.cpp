#include "PreferencesDialog.h"
#include "ui_PreferencesDialog.h"

PreferencesDialog::PreferencesDialog(int chunkSize, int chunkStep, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PreferencesDialog)
{
    ui->setupUi(this);
    ui->chunkSizeSpinBox->setValue(chunkSize);
    ui->chunkStepSpinBox->setValue(chunkStep);
}

PreferencesDialog::~PreferencesDialog()
{
    delete ui;
}

int PreferencesDialog::getChunkSize() const
{
    return ui->chunkSizeSpinBox->value();
}

int PreferencesDialog::getChunkStep() const
{
    return ui->chunkStepSpinBox->value();
}
