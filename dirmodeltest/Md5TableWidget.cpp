#include "Md5TableWidget.h"
#include "ui_Md5TableWidget.h"

#include <QCompleter>
#include <QDesktopServices>
#include <QScrollBar>
#include <QStandardPaths>
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
    auto pictLocationList = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);
    if (!pictLocationList.isEmpty())
    {
        m_ui->pathLineEdit->setText(pictLocationList.first());
    }
    m_model->setRootPath("");
    m_ui->tableView->setModel(m_model);
    m_ui->tableView->setRootIndex(
                m_model->index(m_ui->pathLineEdit->text()));

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
    QCompleter* completer = new QCompleter(m_model, m_ui->pathLineEdit);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    m_ui->pathLineEdit->setCompleter(completer);

    connect(m_ui->pathLineEdit, &QLineEdit::returnPressed,
            this, &Md5TableWidget::cd);

    connect(m_ui->tableView->verticalScrollBar(), &QScrollBar::valueChanged,
            this, &Md5TableWidget::dataChanged);
    connect(m_ui->tableView->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &Md5TableWidget::dataChanged);
    connect(m_ui->tableView->horizontalHeader(), &QHeaderView::sortIndicatorChanged,
            this, &Md5TableWidget::dataChanged);
    setScrollBarPosition(POSITION_RIGHT);
    m_ui->tableView->verticalHeader()->setSortIndicator(0, Qt::AscendingOrder);
    m_ui->tableView->sortByColumn(
                m_ui->tableView->verticalHeader()->sortIndicatorSection(),
                m_ui->tableView->verticalHeader()->sortIndicatorOrder());
}

Md5TableWidget::~Md5TableWidget()
{
    m_md5Worker->computeMd5({});
    m_md5Thread->quit();
    m_md5Thread->wait(3000);
    delete m_md5Thread;
    delete m_ui;
}

void Md5TableWidget::setScrollBarPosition(Md5TableWidget::ScrollBarPosition pos)
{
    switch(pos)
    {
    case POSITION_LEFT:
        m_scrollBar = m_ui->leftVerticalScrollBar;
        break;
    case POSITION_RIGHT:
        m_scrollBar = m_ui->rightVerticalScrollBar;
        break;
    }
    m_ui->leftVerticalScrollBar->setVisible(
                m_ui->leftVerticalScrollBar == m_scrollBar);
    m_ui->rightVerticalScrollBar->setVisible(
                m_ui->rightVerticalScrollBar == m_scrollBar);

    m_scrollBar->setRange(
                m_ui->tableView->verticalScrollBar()->minimum(),
                m_ui->tableView->verticalScrollBar()->maximum());
    m_scrollBar->setValue(m_ui->tableView->verticalScrollBar()->value());
    connect(m_ui->tableView->verticalScrollBar(), &QAbstractSlider::rangeChanged,
            m_scrollBar, &QAbstractSlider::setRange,
            Qt::UniqueConnection);
    connect(m_scrollBar, &QAbstractSlider::valueChanged,
            m_ui->tableView->verticalScrollBar(), &QAbstractSlider::setValue,
            Qt::UniqueConnection);
}

int Md5TableWidget::getTableTopPos() const
{
    const int tableTopPos = m_ui->tableView->geometry().top() +
            m_ui->tableView->horizontalHeader()->height();
    return tableTopPos;
}

QHash<QByteArray, QSet<int>> Md5TableWidget::getMd5PositionHash() const
{
    QHash<QByteArray, QSet<int>> md5PositionHash;

    QModelIndex rootIndex = m_ui->tableView->rootIndex();
    for(int row = 0; row < m_model->rowCount(rootIndex); ++row)
    {
        QModelIndex rowIndex = m_model->index(row, 0, rootIndex);
        QByteArray md5 = m_model->getMd5(rowIndex);
        if (md5.isEmpty())
        {
            continue;
        }
        QRect rowRect = m_ui->tableView->visualRect(rowIndex);
        int rowPosY = rowRect.top() + rowRect.height() / 2;
        md5PositionHash[md5].insert(rowPosY);
    }

    return md5PositionHash;
}

QSet<QByteArray> Md5TableWidget::getSelectedMd5Set() const
{
    QSet<QByteArray> selectedSet;
    auto indexList = m_ui->tableView->selectionModel()->selectedRows();
    for(auto index : indexList)
    {
        QByteArray md5 = m_model->getMd5(index);
        selectedSet.insert(md5);
    }
    return selectedSet;
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

    emit dataChanged();
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

    emit dataChanged();
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

    emit dataChanged();
}

void Md5TableWidget::updatePathLine()
{
    m_ui->pathLineEdit->setText(QDir::toNativeSeparators(m_model->filePath(m_ui->tableView->rootIndex())));
}

void Md5TableWidget::resizeSections()
{
    m_ui->tableView->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
}
