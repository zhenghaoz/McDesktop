#include "mainwindow.h"

#include <QApplication>
#include <QDesktopServices>
#include <QDesktopWidget>
#include <QDir>
#include <QFileDialog>
#include <QUrl>
#include <QVBoxLayout>
#include <QTimer>
#include <QCloseEvent>
#include <QSettings>
#include <chrono>
#include <spdlog/spdlog.h>
#include "heic.h"

using namespace std;

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
{
    // Main window properties
    setFixedSize(QSize(800, 600));
    setWindowTitle("SunDesktop");
    setWindowIcon(QIcon(QPixmap("../assets/icon.png")));

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
    nameLabel = new QLabel("Catalina");
    optionLayout->addWidget(nameLabel);
    modeComboBox = new QComboBox();
    QStringList options;
    options.append("Dynamic");
    options.append("Light");
    options.append("Dark");
    modeComboBox->addItems(options);
    optionLayout->addWidget(modeComboBox);
    hintLabel = new QLabel("");
    optionLayout->addWidget(hintLabel);
    previewLayout->addLayout(optionLayout);
    previewLayout->addStretch();

    // Gallery
    galleryList = new QListWidget();
    galleryList->setViewMode(QListWidget::IconMode);
    galleryList->setIconSize(QSize(178, 178));
    galleryList->setSpacing(3);
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

    // Register callback
    Cache& cache = Cache::getInstance();
    cache.ListenOnCacheChange([this](){
        LoadGallery();
    });

    MoveCenter();
    LoadGallery();
}

MainWindow::~MainWindow()
{
}

void MainWindow::LoadGallery()
{
    Cache& cache = Cache::getInstance();
    pictures = cache.GetCachedPictures();
    galleryList->clear();
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
    Cache& cache = Cache::getInstance();
    // Load default directory
    QSettings settings;
    const QString& dir = settings.value("dir", cache.GetHomeDir()).toString();
    // Open file dialog
    QString file1Name = QFileDialog::getOpenFileName(this, tr("Open HEIC File"), dir, tr("HEIC Files (*.heic)"));
    if (!file1Name.isEmpty()) {
        spdlog::info("add new wallpaper {}", file1Name.toStdString());
        // Save default directory
        QFileInfo fileInfo(file1Name);
        settings.setValue("dir", fileInfo.dir().path());
        // Copy
        const QString& dest = cache.GetPictureDir() + "/" + fileInfo.fileName();
        if (QFile::copy(file1Name, dest)) {
            spdlog::info("add new wallpaper success");
            // Refresh gallery
            QListWidgetItem *item = new QListWidgetItem();
            item->setIcon(QIcon(QPixmap("../assets/loading.jpg").scaled(Heic::kThumbWidth, Heic::kThumbHeight)));
            galleryList->addItem(item);
            cache.NotifyCacheSyncer();
        } else {
            spdlog::info("add new wallpaper failed");
        }
    }
}

void MainWindow::RemoveWallpaper()
{
    Cache& cache = Cache::getInstance();
    const QString& dest = cache.GetPictureDir() + "/" + pictures[selected].name + ".heic";
    if (QFile::remove(dest)) {
        spdlog::info("remove wallpaper succed");
        selected = -1;
        QListWidgetItem* item = galleryList->item(selected);
        item->setIcon(QIcon(QPixmap("../assets/loading.jpg").scaled(Heic::kThumbWidth, Heic::kThumbHeight)));
        cache.NotifyCacheSyncer();
    } else {
        spdlog::info("remove wallpaper failed");
    }
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
    Cache& cache = Cache::getInstance();
    const CachedFrame& frame = pictures[selected].GetFrame(cache.GetCachedLocation(), time);
    imageLabel->setPixmap(frame.thumb.scaled(200, 200, Qt::KeepAspectRatio));

    // Set settings
    cache.SetCurrentDesktop(pictures[selected].name);
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
        const Cache& cache = Cache::getInstance();
        const CachedFrame& frame = pictures[selected].GetFrame(cache.GetCachedLocation(), time);
        imageLabel->setPixmap(frame.thumb.scaled(200, 200, Qt::KeepAspectRatio));
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    hide();
    event->ignore();
}
