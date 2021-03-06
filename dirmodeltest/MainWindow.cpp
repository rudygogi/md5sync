#include "MainWindow.h"
#include "ui_MainWindow.h"

#include "PreferencesDialog.h"
#include "Settings.h"
#include <QTimer>
#include <QSettings>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->leftTable->setScrollBarPosition(Md5TableWidget::POSITION_LEFT);

    QPalette graphPalette = ui->graphicsView->palette();
    graphPalette.setColor(QPalette::Base, graphPalette.color(QPalette::Window));
    ui->graphicsView->setPalette(graphPalette);

    connect(ui->leftTable, &Md5TableWidget::dataChanged, this, [this]() { updateConnections();});
    connect(ui->rightTable, &Md5TableWidget::dataChanged, this, [this]() { updateConnections();});

    connect(ui->preferencesAction, &QAction::triggered, this, [this](bool){ showPreferences(); });
    connect(ui->exitAction, &QAction::triggered, this, [](bool){ QCoreApplication::quit(); });

    Settings settings;
    ui->leftTable->setRootPath(settings.getLeftPath());
    ui->rightTable->setRootPath(settings.getRightPath());
    updatePreferences();
}

MainWindow::~MainWindow()
{
    Settings settings;
    settings.setLeftPath(ui->leftTable->getRootPath());
    settings.setRightPath(ui->rightTable->getRootPath());
    delete ui;
}

void MainWindow::resizeEvent(QResizeEvent* re)
{
    QMainWindow::resizeEvent(re);

    QTimer::singleShot(0, this, [this]() {
        updateConnections();
    });
}

void MainWindow::updateConnections()
{
    QGraphicsScene* scene = new QGraphicsScene(ui->graphicsView);
    scene->setSceneRect(QRect(QPoint(0,-ui->leftTable->getTableTopPos()), ui->graphicsView->size()));

    QHash<QByteArray, QSet<int>> leftPositionHash =
            ui->leftTable->getMd5PositionHash();
    QSet<QByteArray> leftSelectedMd5Set = ui->leftTable->getSelectedMd5Set();

    QHash<QByteArray, QSet<int>> rightPositionHash =
            ui->rightTable->getMd5PositionHash();
    QSet<QByteArray> rightSelectedMd5Set = ui->rightTable->getSelectedMd5Set();

    for(auto leftIt = leftPositionHash.cbegin(); leftIt != leftPositionHash.cend(); ++leftIt)
    {
        QByteArray md5 = leftIt.key();

        bool isLeftSelected = leftSelectedMd5Set.contains(md5);
        bool isRightSelected = rightSelectedMd5Set.contains(md5);

        QSet<int> leftPosSet = leftIt.value();
        QList<int> leftPosList = leftPosSet.toList();
        std::sort(leftPosList.begin(), leftPosList.end());

        QSet<int> rightPosSet = rightPositionHash[md5];
        QList<int> rightPosList = rightPosSet.toList();
        std::sort(rightPosList.begin(), rightPosList.end());

        for(auto leftPosIt = leftPosList.cbegin(); leftPosIt != leftPosList.cend(); ++leftPosIt)
        {
            auto leftNextPosIt = leftPosIt + 1;
            if(leftNextPosIt != leftPosList.cend())
            {
                QPainterPath painterPath;
                int fromY = *leftPosIt;
                int toY = *leftNextPosIt;
                painterPath.moveTo(0, fromY);
                painterPath.cubicTo(
                            ui->graphicsView->width() * 0.2, fromY,
                            ui->graphicsView->width() * 0.2, toY,
                            0, toY);
                QPen pen = QColor(Qt::red);
                if (isLeftSelected)
                {
                    pen.setWidth(3);
                }
                scene->addPath(painterPath, pen);
            }

            if (rightPosSet.isEmpty())
            {
                continue;
            }

            for (auto rightPosIt = rightPosList.cbegin(); rightPosIt != rightPosList.cend(); ++rightPosIt)
            {
                QPainterPath painterPath;
                int fromY = *leftPosIt;
                int toY = *rightPosIt;
                painterPath.moveTo(0, fromY);
                painterPath.cubicTo(
                            ui->graphicsView->width() * 0.5, fromY,
                            ui->graphicsView->width() * 0.5, toY,
                            ui->graphicsView->width(), toY);
                QPen pen = QColor(Qt::gray);
                if (isLeftSelected || isRightSelected)
                {
                    pen.setWidth(3);
                }
                scene->addPath(painterPath, pen);
            }
        }

        for (auto rightPosIt = rightPosList.cbegin(); rightPosIt != rightPosList.cend(); ++rightPosIt)
        {
            auto rightNextPosIt = rightPosIt + 1;
            if(rightNextPosIt != rightPosList.cend())
            {
                QPainterPath painterPath;
                int fromY = *rightPosIt;
                int toY = *rightNextPosIt;
                painterPath.moveTo(ui->graphicsView->width(), fromY);
                painterPath.cubicTo(
                            ui->graphicsView->width() * 0.8, fromY,
                            ui->graphicsView->width() * 0.8, toY,
                            ui->graphicsView->width(), toY);
                QPen pen = QColor(Qt::red);
                if (isRightSelected)
                {
                    pen.setWidth(3);
                }
                scene->addPath(painterPath, pen);
            }
        }

    }

    delete ui->graphicsView->scene();
    ui->graphicsView->setScene(scene);
    //ui->graphicsView->translate(0, ui->leftTable->getTableTopPos());
    ui->graphicsView->update();

    ui->leftTable->setOtherTableData(ui->rightTable->getDirPath(), ui->rightTable->getMd5Set());
    ui->rightTable->setOtherTableData(ui->leftTable->getDirPath(), ui->leftTable->getMd5Set());
}

void MainWindow::showPreferences()
{
    Settings settings;
    int chunkSize = settings.getChunkSize();
    int chunkStep = settings.getChunkStep();
    PreferencesDialog prefDialog(chunkSize, chunkStep, this);
    if (prefDialog.exec() != QDialog::Accepted)
    {
        return;
    }

    settings.setChunkSize(prefDialog.getChunkSize());
    settings.setChunkStep(prefDialog.getChunkStep());
    updatePreferences();
}

void MainWindow::updatePreferences()
{
    Settings settings;
    int chunkSize = settings.getChunkSize();
    int chunkStep = settings.getChunkStep();
    ui->leftTable->setPreferences(chunkSize, chunkStep);
    ui->rightTable->setPreferences(chunkSize, chunkStep);
}
