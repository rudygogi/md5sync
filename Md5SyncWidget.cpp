#include "Md5SyncWidget.h"
#include "ui_Md5SyncWidget.h"

#include <QCompleter>
#include <QFileDialog>
#include <QFileSystemModel>
#include <QSettings>

#include "FileSystemMd5Model.h"

Md5SyncWidget::Md5SyncWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Md5SyncWidget)
{
    ui->setupUi(this);

    QFileSystemModel* pathCompleterModel = new QFileSystemModel(this);
    pathCompleterModel->setRootPath("");
    //pathCompleterModel->setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
    pathCompleterModel->sort(0);
    QCompleter* pathCompleter = new QCompleter(this);
    pathCompleter->setModel(pathCompleterModel);
    ui->leftPathLineEdit->setCompleter(pathCompleter);
    ui->rightPathLineEdit->setCompleter(pathCompleter);

    connect(ui->leftPathButton, &QPushButton::clicked, [this]() {
        showOpenDirDialog(ui->leftPathLineEdit); });
    connect(ui->rightPathButton, &QPushButton::clicked, [this]() {
        showOpenDirDialog(ui->rightPathLineEdit); });

    restoreSettings(GEOMETRY_SETTING, [this](const QVariant& value) { restoreGeometry(value.toByteArray()); });
    restoreSettings(LEFT_PATH_SETTING, [this](const QVariant& value) { ui->leftPathLineEdit->setText(value.toString());});
    restoreSettings(RIGHT_PATH_SETTING, [this](const QVariant& value) { ui->rightPathLineEdit->setText(value.toString());});

    FileSystemMd5Model* fileModel = new FileSystemMd5Model(this);
    fileModel->setRootPath("");
    fileModel->setFilter(QDir::AllEntries | QDir::NoDot);
    ui->leftTableView->setModel(fileModel);
    ui->rightTableView->setModel(fileModel);
    updatePath(ui->leftPathLineEdit, ui->leftTableView);
    updatePath(ui->rightPathLineEdit, ui->rightTableView);

    ui->leftTableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
}

Md5SyncWidget::~Md5SyncWidget()
{
    QSettings settings;

    settings.setValue(GEOMETRY_SETTING, saveGeometry());
    settings.setValue(LEFT_PATH_SETTING, ui->leftPathLineEdit->text());
    settings.setValue(RIGHT_PATH_SETTING, ui->rightPathLineEdit->text());

    delete ui;
}

void Md5SyncWidget::showOpenDirDialog(QLineEdit *dirLineEdit)
{
    QString newDir = QFileDialog::getExistingDirectory(this, "Choose a directory to compare", dirLineEdit->text());
    if (newDir.isNull())
    {
        return;
    }
    dirLineEdit->setText(newDir);    
}

void Md5SyncWidget::restoreSettings(const QString &settingsName, std::function<void (const QVariant &)> setFunction)
{
    QSettings settings;
    QVariant value = settings.value(settingsName);
    if (value.isNull())
    {
        return;
    }
    setFunction(settings.value(settingsName));
}

void Md5SyncWidget::updatePath(QLineEdit *pathLineEdit, QAbstractItemView *itemView)
{
    QFileSystemModel* model = qobject_cast<QFileSystemModel*>(itemView->model());
    QModelIndex pathIndex = model->index(pathLineEdit->text());
    itemView->setRootIndex(pathIndex);
}
