#include "Widget.h"
#include "ui_Widget.h"

#include <QDateTime>
#include <QDir>
#include <QFileDialog>
#include <QGraphicsScene>
#include <QSettings>
#include <QFileInfo>
#include <QFile>
#include <QCryptographicHash>
#include <QPainterPath>
#include <QScrollBar>
#include <QDebug>

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    QPalette graphPalette = ui->graphicsView->palette();
    graphPalette.setColor(QPalette::Base, graphPalette.color(QPalette::Window));
    ui->graphicsView->setPalette(graphPalette);

    ui->leftTreeWidget->setLayoutDirection(Qt::RightToLeft);
    ui->leftTreeWidget->header()->setLayoutDirection(Qt::LeftToRight);

    QSettings settings;

    QByteArray winGeometry = settings.value("geometry").toByteArray();
    if (!winGeometry.isEmpty())
    {
        restoreGeometry(winGeometry);
    }

    QString leftPath = settings.value("leftpath").toString();
    if (leftPath.isEmpty()) {
        leftPath = QDir::currentPath();
    }
    ui->leftPathLineEdit->setText(QDir::toNativeSeparators(leftPath));

    QString rightPath = settings.value("rightpath").toString();
    if (rightPath.isEmpty()) {
        rightPath = QDir::currentPath();
    }
    ui->rightPathLineEdit->setText(QDir::toNativeSeparators(rightPath));

    QString filter = settings.value("filter").toString();
    if (!filter.isEmpty())
    {
        ui->filterComboBox->setCurrentText(filter);
    }

    connect(ui->leftPathButton, &QPushButton::clicked, this, [&]() {
        QString path = QFileDialog::getExistingDirectory(this, "Choose a directory", ui->leftPathLineEdit->text());
        if (path.isEmpty()) {
            return;
        }
        ui->leftPathLineEdit->setText(QDir::toNativeSeparators(path));
    });

    connect(ui->rightPathButton, &QPushButton::clicked, this, [&]() {
        QString path = QFileDialog::getExistingDirectory(this, "Choose a directory", ui->rightPathLineEdit->text());
        if (path.isEmpty()) {
            return;
        }
        ui->rightPathLineEdit->setText(QDir::toNativeSeparators(path));
    });

    connect(ui->compareButton, &QPushButton::clicked, this, &Widget::compare);

    ui->graphicsView->installEventFilter(this);
    connect(ui->leftTreeWidget->verticalScrollBar(), &QScrollBar::valueChanged,
            this, [&](int){updateConnections();});
    connect(ui->rightTreeWidget->verticalScrollBar(), &QScrollBar::valueChanged,
            this, [&](int){updateConnections();});

    connect(ui->selectButton, &QPushButton::clicked, this, &Widget::selectMissing);

    //compare();

    connect(ui->leftTreeWidget->header(), &QHeaderView::sortIndicatorChanged,
            this, [&](int, Qt::SortOrder){updateConnections();}, Qt::QueuedConnection);

    connect(ui->rightTreeWidget->header(), &QHeaderView::sortIndicatorChanged,
            this, [&](int, Qt::SortOrder){updateConnections();}, Qt::QueuedConnection);

    static bool updatingSelection = false;
    connect(ui->leftTreeWidget->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, [&](const QItemSelection&,const QItemSelection&) {
        if(updatingSelection)
        {
            return;
        }
        updatingSelection = true;
        QModelIndexList indexList = ui->leftTreeWidget->selectionModel()->selectedRows(0);
        QSet<int> rightRowSet;
        for(auto index : indexList)
        {
            QString md5Hash = index.data(Qt::DisplayRole).toString();
            rightRowSet += getRightRowSet(md5Hash);
        }
        ui->rightTreeWidget->selectionModel()->clear();
        for(int rightRow : rightRowSet)
        {
            QModelIndex rightIndex = ui->rightTreeWidget->model()->index(rightRow, 0);
            ui->rightTreeWidget->selectionModel()->select(rightIndex, QItemSelectionModel::Select |QItemSelectionModel::Rows);
        }
        updatingSelection = false;
        updateConnections();
    });

    connect(ui->rightTreeWidget->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, [&](const QItemSelection&,const QItemSelection&) {
        if(updatingSelection)
        {
            return;
        }
        updatingSelection = true;
        QModelIndexList indexList = ui->rightTreeWidget->selectionModel()->selectedRows(0);
        QSet<int> leftRowSet;
        for(auto index : indexList)
        {
            QString md5Hash = index.data(Qt::DisplayRole).toString();
            leftRowSet += getLeftRowSet(md5Hash);
        }
        ui->leftTreeWidget->selectionModel()->clear();
        for(int leftRow : leftRowSet)
        {
            QModelIndex leftIndex = ui->leftTreeWidget->model()->index(leftRow, 0);
            ui->leftTreeWidget->selectionModel()->select(leftIndex, QItemSelectionModel::Select |QItemSelectionModel::Rows);
        }
        updatingSelection = false;
        updateConnections();
    });

}

Widget::~Widget()
{
    QSettings settings;
    settings.setValue("geometry", saveGeometry());
    settings.setValue("leftpath", ui->leftPathLineEdit->text());
    settings.setValue("rightpath", ui->rightPathLineEdit->text());
    settings.setValue("filter", ui->filterComboBox->currentText());

    delete ui;
}

bool Widget::eventFilter(QObject *watched, QEvent *event)
{
    if ((watched == ui->graphicsView) && (event->type() == QEvent::Resize))
    {
        updateConnections();
    }
    return QWidget::eventFilter(watched, event);
}

void Widget::compare()
{
    ui->leftTreeWidget->clear();
    QDir leftDir(ui->leftPathLineEdit->text());
    QFileInfoList leftFileInfoList = leftDir.entryInfoList(ui->filterComboBox->currentText().split(';'), QDir::Files | QDir::NoDotAndDotDot);
    for(const auto& fileInfo : leftFileInfoList)
    {
        QString md5;
        QCryptographicHash md5Hash(QCryptographicHash::Md5);
        QFile file(fileInfo.absoluteFilePath());
        if (file.open(QIODevice::ReadOnly))
        {
            md5Hash.addData(file.readAll());
            md5 = md5Hash.result().toHex();
        }
        QStringList sl({fileInfo.fileName(), QString::number(fileInfo.size()),fileInfo.created().toString(Qt::SystemLocaleShortDate), md5});
        QTreeWidgetItem* item = new QTreeWidgetItem(ui->leftTreeWidget, sl);
        item->setTextAlignment(0, Qt::AlignRight);
        item->setTextAlignment(1, Qt::AlignLeft);
        item->setTextAlignment(2, Qt::AlignLeft);
        item->setTextAlignment(3, Qt::AlignRight);
    }

    ui->rightTreeWidget->clear();
    QDir rightDir(ui->rightPathLineEdit->text());
    QFileInfoList rightFileInfoList = rightDir.entryInfoList(ui->filterComboBox->currentText().split(';'), QDir::Files | QDir::NoDotAndDotDot);
    for(const auto& fileInfo : rightFileInfoList)
    {
        QString md5;
        QCryptographicHash md5Hash(QCryptographicHash::Md5);
        QFile file(fileInfo.absoluteFilePath());
        if (file.open(QIODevice::ReadOnly))
        {
            md5Hash.addData(file.readAll());
            md5 = md5Hash.result().toHex();
        }
        QStringList sl({fileInfo.fileName(), QString::number(fileInfo.size()),fileInfo.created().toString(), md5});
        QTreeWidgetItem* item = new QTreeWidgetItem(ui->rightTreeWidget, sl);
        item->setTextAlignment(0, Qt::AlignLeft);
        item->setTextAlignment(1, Qt::AlignRight);
        item->setTextAlignment(2, Qt::AlignRight);
        item->setTextAlignment(3, Qt::AlignLeft);
    }
    selectMissing();
    updateConnections();
}

void Widget::updateConnections()
{
    QGraphicsScene* scene = new QGraphicsScene(ui->graphicsView);
    scene->setSceneRect(QRect(QPoint(0,0), ui->graphicsView->size()));

    int leftHeaderHeight = ui->leftTreeWidget->header()->height();
    int rightHeaderHeight = ui->rightTreeWidget->header()->height();
    for (int row = 0; row < ui->leftTreeWidget->topLevelItemCount(); ++row) {
        QString md5Hash = ui->leftTreeWidget->topLevelItem(row)->data(3, Qt::DisplayRole).toString();
        QSet<int> rightRowSet = getRightRowSet(md5Hash);
        QModelIndex leftIndex = ui->leftTreeWidget->model()->index(row, 0);
        QRect leftRect = ui->leftTreeWidget->visualRect(leftIndex);
        if (!leftRect.isValid()) {
            continue;
        }
        QPen pen = QColor(Qt::gray);
        for (int rightRow : rightRowSet) {
            QModelIndex rightIndex = ui->rightTreeWidget->model()->index(rightRow, 0);
            QRect rightRect = ui->rightTreeWidget->visualRect(rightIndex);
            if (!rightRect.isValid())
            {
                continue;
            }
            QPainterPath painterPath;
            int fromY = leftHeaderHeight + leftRect.y() + leftRect.height() / 2;
            int toY = rightHeaderHeight + rightRect.y() + rightRect.height() / 2;
            painterPath.moveTo(0, fromY);
            painterPath.cubicTo(
                        ui->graphicsView->width() * 0.5, fromY,
                        ui->graphicsView->width() * 0.5, toY,
                        ui->graphicsView->width(), toY);
            if (ui->leftTreeWidget->selectionModel()->isSelected(leftIndex) ||
                    ui->rightTreeWidget->selectionModel()->isSelected(rightIndex))
            {
                pen = QPen(QColor(0,120,215), 3);
            }
            scene->addPath(painterPath, pen);
        }
    }

    delete ui->graphicsView->scene();
    ui->graphicsView->setScene(scene);
}

void Widget::selectMissing()
{
    ui->leftTreeWidget->selectionModel()->clear();
    for (int row = 0; row < ui->leftTreeWidget->topLevelItemCount(); ++row) {
        QString md5Hash = ui->leftTreeWidget->topLevelItem(row)->data(3, Qt::DisplayRole).toString();
        QSet<int> rightRowSet = getRightRowSet(md5Hash);
        if (!rightRowSet.isEmpty())
        {
            continue;
        }
        QModelIndex leftIndex = ui->leftTreeWidget->model()->index(row, 0);
        ui->leftTreeWidget->selectionModel()->select(leftIndex, QItemSelectionModel::Select | QItemSelectionModel::Rows);
    }
}

QSet<int> Widget::getRightRowSet(const QString &md5Hash) const
{
    QSet<int> rowSet;
    for (int row = 0; row < ui->rightTreeWidget->topLevelItemCount(); ++row) {
        QString rightMd5Hash = ui->rightTreeWidget->topLevelItem(row)->data(3, Qt::DisplayRole).toString();
        if (rightMd5Hash == md5Hash)
        {
            rowSet.insert(row);
        }
    }
    return rowSet;
}
QSet<int> Widget::getLeftRowSet(const QString &md5Hash) const
{
    QSet<int> rowSet;
    for (int row = 0; row < ui->leftTreeWidget->topLevelItemCount(); ++row) {
        QString leftMd5Hash = ui->leftTreeWidget->topLevelItem(row)->data(3, Qt::DisplayRole).toString();
        if (leftMd5Hash == md5Hash)
        {
            rowSet.insert(row);
        }
    }
    return rowSet;
}
