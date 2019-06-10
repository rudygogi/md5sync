#include "Md5TableWidget.h"
#include "ui_Md5TableWidget.h"

#include <QCompleter>
#include <QDesktopServices>
#include <QImageReader>
#include <QScrollBar>
#include <QStandardPaths>
#include <QThread>
#include <QTimer>
#include <QUrl>
#include <QKeyEvent>

#include "Md5Model.h"
#include "Md5Processor.h"
#include "IconProvider.h"

#include <QDesktopServices>
#include <QUrl>

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

    m_iconThread = new QThread(this);
    m_iconWorker = new IconProvider;
    m_iconWorker->moveToThread(m_iconThread);
    connect(m_iconThread, &QThread::finished,
            m_iconWorker, &QObject::deleteLater);
    connect(m_iconWorker, &IconProvider::iconReady,
            this, &Md5TableWidget::showIcon);
    m_iconThread->start();

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

    connect(m_ui->tableView->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, [=](const QItemSelection&, const QItemSelection& ) {
       showSelectedPreviews();
    });

    connect(m_ui->listWidget, &QListWidget::itemDoubleClicked, this, [](QListWidgetItem* item) {
       if (!item)
       {
           return;
       }
       QString absFileName = item->data(Qt::UserRole).toString();
       if (absFileName.isEmpty())
       {
           return;
       }
       QUrl url = QUrl::fromLocalFile(absFileName);
       QDesktopServices::openUrl(url);
    });

    connect(m_model, &QFileSystemModel::directoryLoaded, this, [this](const QString&) {
       onDirectoryLoaded();
    }, Qt::QueuedConnection);
    connect(m_model, &QFileSystemModel::rootPathChanged, this, [this](const QString&) {
       onDirectoryLoaded();
    }, Qt::QueuedConnection);
    connect(m_model, &QAbstractItemModel::layoutChanged, this, [this]() {
       onDirectoryLoaded();
    }, Qt::QueuedConnection);

    connect(m_ui->allButton, &QPushButton::clicked, this, [this]() {
        selectAll();
    });
    connect(m_ui->duplicatesButton, &QPushButton::clicked, this, [this]() {
        selectDuplicates();
    });
    connect(m_ui->presentButton, &QPushButton::clicked, this, [this]() {
        selectPresent();
    });
    connect(m_ui->missingButton, &QPushButton::clicked, this, [this]() {
        selectMissing();
    });

    connect(m_ui->noneButton, &QPushButton::clicked,
            this, [this]() { selectNone(); });

    connect(m_ui->cleanButton, &QPushButton::clicked,
            this, [this]() { cleanFiles(); });
    connect(m_ui->copyButton, &QPushButton::clicked,
            this, [this]() { copyFiles(); });
    connect(m_ui->moveButton, &QPushButton::clicked,
            this, [this]() { moveFiles(); });

    m_ui->tableView->installEventFilter(this);
}

Md5TableWidget::~Md5TableWidget()
{
    m_md5Worker->computeMd5({});
    m_md5Thread->quit();
    m_md5Thread->wait(3000);
    delete m_md5Thread;
    delete m_ui;
}

void Md5TableWidget::setRootPath(const QString &path)
{
    if (path.isEmpty())
    {
        return;
    }
    if (!QDir(path).exists())
    {
        return;
    }
    m_ui->pathLineEdit->setText(path);
    cd();
}

QString Md5TableWidget::getRootPath() const
{
    auto rootIndex = m_ui->tableView->rootIndex();
    auto pathInfo = m_model->fileInfo(rootIndex);
    return pathInfo.absoluteFilePath();
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

int Md5TableWidget::getTableHeight() const
{
    const int tableHeight = m_ui->tableView->geometry().height() -
            m_ui->tableView->horizontalHeader()->height();
    return tableHeight;
}

void Md5TableWidget::setPreferences(int chunkSize, int chunkStep)
{
    m_md5Worker->setPreferences(chunkSize, chunkStep);
    m_model->resetMd5Hash();
    requestMd5();
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

QString Md5TableWidget::getDirPath() const
{
    auto rootIndex = m_ui->tableView->rootIndex();
    auto pathInfo = m_model->fileInfo(rootIndex);
    return pathInfo.absoluteFilePath();
}

void Md5TableWidget::setOtherTableData(const QString& dirPath, const QSet<QByteArray>& md5Set)
{
    m_otherDirPath = dirPath;
    m_otherMd5Set = md5Set;
}

QSet<QByteArray> Md5TableWidget::getMd5Set() const
{
    QSet<QByteArray> md5Set;
    QModelIndex rootIndex = m_ui->tableView->rootIndex();
    for(int row = 0; row < m_model->rowCount(rootIndex); ++row)
    {
        QModelIndex index = rootIndex.child(row, 0);
        QByteArray md5 = m_model->getMd5(index);
        if (md5.isEmpty())
        {
            continue;
        }
        md5Set.insert(md5);
    }
    return md5Set;
}

bool Md5TableWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_ui->tableView && event->type() == QEvent::KeyPress)
    {
        auto keyEvent = dynamic_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Backspace)
        {
            cdUp();
        }
        else if (keyEvent->key() == Qt::Key_Return)
        {
            onDoubleClicked(m_ui->tableView->currentIndex());
        }
    }
    return QWidget::eventFilter(watched, event);
}

void Md5TableWidget::onDoubleClicked(const QModelIndex &index)
{
    QFileInfo fileInfo = m_model->fileInfo(index);
    if (fileInfo.isDir())
    {
        m_ui->tableView->setRootIndex(m_model->index(fileInfo.absoluteFilePath()));
        onDirectoryLoaded();
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
    auto parentIndex = m_ui->tableView->rootIndex().parent();
    m_ui->tableView->setRootIndex(parentIndex);
    m_ui->tableView->selectionModel()->select(
                parentIndex,
                QItemSelectionModel::Select | QItemSelectionModel::Rows | QItemSelectionModel::Current);
    onDirectoryLoaded();
    showSelectedPreviews();
    m_ui->tableView->scrollTo(m_ui->tableView->currentIndex(), QAbstractItemView::PositionAtCenter);
    m_ui->tableView->setFocus();
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
    onDirectoryLoaded();
    selectNone();
    showSelectedPreviews();
}

void Md5TableWidget::updatePathLine()
{
    m_ui->pathLineEdit->setText(QDir::toNativeSeparators(m_model->filePath(m_ui->tableView->rootIndex())));
}

void Md5TableWidget::onDirectoryLoaded()
{
    QCoreApplication::processEvents(QEventLoop::AllEvents);
    m_ui->tableView->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
    requestMd5();
    updatePathLine();
    resizeSections();

    emit dataChanged();
}

void Md5TableWidget::resizeSections()
{
    m_ui->tableView->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
}

void Md5TableWidget::showSelectedPreviews()
{
    m_ui->listWidget->clear();
    auto selectedRows = m_ui->tableView->selectionModel()->selectedRows();
    QStringList fileNameList;
    for(auto selectedIndex : selectedRows)
    {
        auto fileInfo = m_model->fileInfo(selectedIndex);
        if (fileInfo.isDir())
        {
            continue;
        }

        auto item = new QListWidgetItem(fileInfo.fileName().left(8).append("..."));
        item->setData(Qt::UserRole, fileInfo.absoluteFilePath());
        m_ui->listWidget->addItem(item);
        fileNameList.append(fileInfo.absoluteFilePath());
    }
    m_iconWorker->provideIcons(fileNameList);
}

void Md5TableWidget::showIcon(const QString &fileName, const QIcon &icon)
{
    for(int row = 0; row < m_ui->listWidget->count(); ++row)
    {
        auto* item = m_ui->listWidget->item(row);
        if (!item)
        {
            continue;
        }
        QString itemFileName = item->data(Qt::UserRole).toString();
        if (itemFileName != fileName)
        {
            continue;
        }
        auto newItem = new QListWidgetItem(icon, item->text());
        newItem->setData(Qt::UserRole, item->data(Qt::UserRole));
        delete m_ui->listWidget->takeItem(row);
        m_ui->listWidget->insertItem(row, newItem);
    }
}

void Md5TableWidget::selectAll()
{
    selectNone();
    QModelIndex rootIndex = m_ui->tableView->rootIndex();
    for(int row = 0; row < m_model->rowCount(rootIndex); ++row)
    {
        QModelIndex index = rootIndex.child(row, 0);
        if (m_model->fileInfo(index).isDir())
        {
            continue;
        }
        m_ui->tableView->selectionModel()->select(index, QItemSelectionModel::Select | QItemSelectionModel::Rows);
    }
}

void Md5TableWidget::selectNone()
{
    m_ui->tableView->clearSelection();
}

void Md5TableWidget::selectDuplicates()
{
    selectNone();
    QHash<QByteArray, QModelIndexList> md5IndexHash;
    QModelIndex rootIndex = m_ui->tableView->rootIndex();
    for(int row = 0; row < m_model->rowCount(rootIndex); ++row)
    {
        QModelIndex index = rootIndex.child(row, 0);
        QByteArray md5 = m_model->getMd5(index);
        if (md5.isEmpty())
        {
            continue;
        }
        md5IndexHash[md5].append(index);
    }
    for(auto md5 : md5IndexHash.keys())
    {
        const auto& indexList = md5IndexHash[md5];
        if (indexList.size() == 1)
        {
            continue;
        }
        for(auto index : indexList)
        {
            m_ui->tableView->selectionModel()->select(index, QItemSelectionModel::Select | QItemSelectionModel::Rows);
        }
    }
}

void Md5TableWidget::selectPresent()
{
    selectNone();
    QModelIndex rootIndex = m_ui->tableView->rootIndex();
    for(int row = 0; row < m_model->rowCount(rootIndex); ++row)
    {
        QModelIndex index = rootIndex.child(row, 0);
        QByteArray md5 = m_model->getMd5(index);
        if (!md5.isEmpty() && m_otherMd5Set.contains(md5))
        {
            m_ui->tableView->selectionModel()->select(index, QItemSelectionModel::Select | QItemSelectionModel::Rows);
        }
    }
}

void Md5TableWidget::selectMissing()
{
    selectNone();
    QModelIndex rootIndex = m_ui->tableView->rootIndex();
    for(int row = 0; row < m_model->rowCount(rootIndex); ++row)
    {
        QModelIndex index = rootIndex.child(row, 0);
        QByteArray md5 = m_model->getMd5(index);
        if (!md5.isEmpty() && !m_otherMd5Set.contains(md5))
        {
            m_ui->tableView->selectionModel()->select(index, QItemSelectionModel::Select | QItemSelectionModel::Rows);
        }
    }
}

void Md5TableWidget::cleanFiles()
{
    auto rootIndex = m_ui->tableView->rootIndex();
    auto pathInfo = m_model->fileInfo(rootIndex);
    if (!pathInfo.isDir())
    {
        return;
    }
    QString cleanDirName = QFileInfo(pathInfo.absoluteFilePath(), "cleaned").absoluteFilePath();
    if (!QDir(cleanDirName).exists())
    {
        QDir(pathInfo.absoluteFilePath()).mkdir("cleaned");
    }
    for(int row = 0; row < m_ui->listWidget->count(); ++row)
    {
        auto* item = m_ui->listWidget->item(row);
        QString oldFilePath = item->data(Qt::UserRole).toString();
        QString newFilePath = QFileInfo(cleanDirName, QFileInfo(oldFilePath).fileName()).absoluteFilePath();
        QFile::rename(oldFilePath, newFilePath);
    }
}

void Md5TableWidget::copyFiles()
{
    auto rootIndex = m_ui->tableView->rootIndex();
    auto pathInfo = m_model->fileInfo(rootIndex);
    if (!pathInfo.isDir())
    {
        return;
    }
    for(int row = 0; row < m_ui->listWidget->count(); ++row)
    {
        auto* item = m_ui->listWidget->item(row);
        QString oldFilePath = item->data(Qt::UserRole).toString();
        QString newFilePath = QFileInfo(m_otherDirPath, QFileInfo(oldFilePath).fileName()).absoluteFilePath();
        QFile::copy(oldFilePath, newFilePath);
    }
}

void Md5TableWidget::moveFiles()
{
    auto rootIndex = m_ui->tableView->rootIndex();
    auto pathInfo = m_model->fileInfo(rootIndex);
    if (!pathInfo.isDir())
    {
        return;
    }
    for(int row = 0; row < m_ui->listWidget->count(); ++row)
    {
        auto* item = m_ui->listWidget->item(row);
        QString oldFilePath = item->data(Qt::UserRole).toString();
        QString newFilePath = QFileInfo(m_otherDirPath, QFileInfo(oldFilePath).fileName()).absoluteFilePath();
        QFile::rename(oldFilePath, newFilePath);
    }
}
