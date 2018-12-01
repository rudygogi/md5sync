#include "Md5TableWidget.h"
#include "ui_Md5TableWidget.h"

#include <QCompleter>
#include <QDesktopServices>
#include <QThread>
#include <QTimer>
#include <QUrl>

#include "Md5Model.h"
#include "Md5Processor.h"

Md5TableWidget::Md5TableWidget(QWidget *parent) :
    QWidget(parent),
    m_ui(new Ui::Md5TableWidget)
{
    m_ui->setupUi(this);

    m_ui->tableView->verticalHeader()->setDefaultSectionSize(
                m_ui->tableView->fontMetrics().height());

    m_model = new Md5Model(this);
    m_model->setRootPath("/");
    m_ui->tableView->setModel(m_model);

    connect(m_ui->tableView, &QAbstractItemView::doubleClicked,
            this, &Md5TableWidget::onDoubleClicked);

    m_md5Thread = new QThread(this);
    m_md5Worker = new Md5Processor;
    m_md5Worker->moveToThread(m_md5Thread);
    connect(m_md5Thread, &QThread::finished,
            m_md5Worker, &QObject::deleteLater);
    connect(m_md5Worker, &Md5Processor::md5Computed,
            this, &Md5TableWidget::setMd5);
    m_md5Thread->start();
    requestMd5();

    connect(m_model, &QAbstractItemModel::rowsInserted,
            this, &Md5TableWidget::requestMd5);
    connect(m_model, &QAbstractItemModel::rowsInserted,
            this, [=](){ resizeSections(); });
    connect(m_ui->cdUpButton, &QToolButton::clicked,
            this, &Md5TableWidget::cdUp);
    updatePathLine();
    QCompleter* completer = new QCompleter(m_model, m_ui->pathLineEdit);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    m_ui->pathLineEdit->setCompleter(completer);

    connect(m_ui->pathLineEdit, &QLineEdit::returnPressed,
            this, &Md5TableWidget::cd);
}

Md5TableWidget::~Md5TableWidget()
{
    m_md5Worker->computeMd5({});
    m_md5Thread->quit();
    m_md5Thread->wait(3000);
    delete m_md5Thread;
    delete m_ui;
}

void Md5TableWidget::onDoubleClicked(const QModelIndex &index)
{
    QFileInfo fileInfo = m_model->fileInfo(index);
    if (fileInfo.isDir())
    {
        m_ui->tableView->setRootIndex(m_model->index(fileInfo.absoluteFilePath()));
        resizeSections();
        updatePathLine();
        requestMd5();
    }
    else
    {
        QDesktopServices::openUrl(QUrl::fromLocalFile(fileInfo.absoluteFilePath()));
    }
}

void Md5TableWidget::setMd5(const QString &fileInfo, const QByteArray &md5)
{
    QModelIndex index = m_model->index(fileInfo);
    m_model->setMd5(index, md5);
}

void Md5TableWidget::requestMd5()
{
    QStringList fileInfoList = m_model->getMissingMd5FileInfoList(m_ui->tableView->rootIndex());
    QTimer::singleShot(0, m_md5Worker, [=](){ m_md5Worker->computeMd5(fileInfoList); });
}

void Md5TableWidget::cdUp()
{
    m_ui->tableView->setRootIndex(m_ui->tableView->rootIndex().parent());
    requestMd5();
    updatePathLine();
    resizeSections();
}

void Md5TableWidget::cd()
{
    QString path = m_ui->pathLineEdit->text();
    QModelIndex index = m_model->index(path);
    if (!index.isValid() || !m_model->isDir(index))
    {
        return;
    }
    m_ui->tableView->setRootIndex(index);
    requestMd5();
    m_ui->tableView->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
}

void Md5TableWidget::updatePathLine()
{
    m_ui->pathLineEdit->setText(QDir::toNativeSeparators(m_model->filePath(m_ui->tableView->rootIndex())));
}

void Md5TableWidget::resizeSections()
{
    m_ui->tableView->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
}
