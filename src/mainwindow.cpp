#include "mainwindow.h"

#include <QApplication>
#include <QDesktopServices>
#include <QDesktopWidget>
#include <QDir>
#include <QFileDialog>
#include <QUrl>
#include <QVBoxLayout>
#include <QTimer>
#include <chrono>
#include <spdlog/spdlog.h>
#include "heic.h"

using namespace std;

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
{
    // Main window properties
    setFixedSize(QSize(800, 600));
    setWindowTitle("动态桌面");

    // Create base layout
    QVBoxLayout *baseLayout = new QVBoxLayout();
    setLayout(baseLayout);

    // Preview
    QHBoxLayout *previewLayout = new QHBoxLayout();
    previewLayout->setContentsMargins(32, 32, 32, 32);
    baseLayout->addLayout(previewLayout);
    imageLabel = new QLabel();
    imageLabel->setPixmap(QPixmap("../images/sample.jpeg").scaled(200, 200, Qt::KeepAspectRatio));
    imageLabel->setContentsMargins(8, 8, 8, 8);
    imageLabel->setStyleSheet("border: 3px solid white;");
    previewLayout->addWidget(imageLabel);
    QVBoxLayout *optionLayout = new QVBoxLayout();
    optionLayout->setContentsMargins(32, 10, 10, 10);
    nameLabel = new QLabel("卡特琳娜岛");
    optionLayout->addWidget(nameLabel);
    modeComboBox = new QComboBox();
    QStringList options;
    options.append("动态");
    options.append("动态");
    options.append("动态");
    modeComboBox->addItems(options);
    optionLayout->addWidget(modeComboBox);
    hintLabel = new QLabel("此桌面图片会基于你的位置不断更改");
    optionLayout->addWidget(hintLabel);
    previewLayout->addLayout(optionLayout);
    previewLayout->addStretch();

    // Gallery
    galleryList = new QListWidget();
    galleryList->setViewMode(QListWidget::IconMode);
    galleryList->setIconSize(QSize(180, 180));
    galleryList->setSpacing(2);
    galleryList->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    connect(galleryList, &QListWidget::itemClicked, this, &MainWindow::SelectPicture);
    baseLayout->addWidget(galleryList);

    // Bottom
    QHBoxLayout *bottomLayout = new QHBoxLayout();
    baseLayout->addLayout(bottomLayout);
    addButton = new QPushButton();
    addButton->setIcon(QIcon(QPixmap("../assets/add.png")));
    addButton->setFixedSize(32, 32);
    connect(addButton, &QPushButton::clicked, this, &MainWindow::AddWallpaper);
    bottomLayout->addWidget(addButton);
    deleteButton = new QPushButton();
    deleteButton->setIcon(QIcon(QPixmap("../assets/delete.png")));
    deleteButton->setFixedSize(32, 32);
    connect(deleteButton, &QPushButton::clicked, this, &MainWindow::RemoveWallpaper);
    bottomLayout->addWidget(deleteButton);
    bottomLayout->addStretch();
    githubButton = new QPushButton();
    githubButton->setIcon(QIcon(QPixmap("../assets/github.png")));
    githubButton->setFixedSize(32, 32);
    connect(githubButton, &QPushButton::clicked, this, &MainWindow::OpenGitHub);
    bottomLayout->addWidget(githubButton);

    // Preview timer
    QTimer *previewTimer = new QTimer(this);
    connect(previewTimer, &QTimer::timeout, this, &MainWindow::PlayPreview);
    previewTimer->start(1000);

    MoveCenter();
    LoadGallery();
}

MainWindow::~MainWindow()
{
}

void MainWindow::LoadGallery()
{
    pictures = cache.ListCachedPictures();
    for(const CachedPicture picture : pictures) {
        QListWidgetItem *item = new QListWidgetItem();
        item->setIcon(QIcon(picture.cover));
        galleryList->addItem(item);
    }
}

void MainWindow::MoveCenter()
{
    QRect desktopRect = QApplication::desktop()->availableGeometry(this);
    QPoint center = desktopRect.center();
    move(center.x() - width() * 0.5, center.y() - height() * 0.5);
}

void MainWindow::OpenGitHub()
{
    QDesktopServices::openUrl(QUrl(QString("https://github.com/zhenghaoz/dynamic-desktop")));
}

void MainWindow::AddWallpaper()
{
    QString file1Name = QFileDialog::getOpenFileName(this, tr("Open XML File 1"), "/home", tr("XML Files (*.xml)"));
}

void MainWindow::RemoveWallpaper()
{

}

void MainWindow::SelectPicture(QListWidgetItem* item)
{
    const QModelIndexList indices = galleryList->selectionModel()->selectedIndexes();
    const QModelIndex index = indices.front();
    play = 0;
    selected = index.row();
    spdlog::info("picture {} selected", selected);

    nameLabel->setText(pictures[selected].name);

    auto currentTime = chrono::system_clock::now();
    time_t tt = chrono::system_clock::to_time_t(currentTime);
    tm local_tm = *gmtime(&tt);
    spdlog::info("H {}", local_tm.tm_hour);
    Time time;
    time.year = local_tm.tm_year + 1900;
    time.month = local_tm.tm_mon + 1;
    time.day = local_tm.tm_mday;
    time.hour = play;
    time.minute = local_tm.tm_min;
    time.second = local_tm.tm_sec;
    const CachedFrame& frame = pictures[selected].GetFrame(cache.GetCachedLocation(), time);
    imageLabel->setPixmap(frame.thumb.scaled(200, 200, Qt::KeepAspectRatio));
}

void MainWindow::PlayPreview()
{
    if (selected >= 0) {
        play = (play + 1) % 24;
        auto currentTime = chrono::system_clock::now();
        time_t tt = chrono::system_clock::to_time_t(currentTime);
        tm local_tm = *gmtime(&tt);
        spdlog::info("H {}", local_tm.tm_hour);
        Time time;
        time.year = local_tm.tm_year + 1900;
        time.month = local_tm.tm_mon + 1;
        time.day = local_tm.tm_mday;
        time.hour = play;
        time.minute = local_tm.tm_min;
        time.second = local_tm.tm_sec;
        const CachedFrame& frame = pictures[selected].GetFrame(cache.GetCachedLocation(), time);
        imageLabel->setPixmap(frame.thumb.scaled(200, 200, Qt::KeepAspectRatio));
    }
}
