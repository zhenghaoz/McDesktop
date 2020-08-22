#include "mainwindow.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QVBoxLayout>

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
    baseLayout->addWidget(galleryList);

    // Bottom
    QHBoxLayout *bottomLayout = new QHBoxLayout();
    baseLayout->addLayout(bottomLayout);
    addButton = new QPushButton();
    addButton->setIcon(QIcon(QPixmap("../assets/add.png")));
    addButton->setFixedSize(32, 32);
    bottomLayout->addWidget(addButton);
    deleteButton = new QPushButton();
    deleteButton->setIcon(QIcon(QPixmap("../assets/delete.png")));
    deleteButton->setFixedSize(32, 32);
    bottomLayout->addWidget(deleteButton);
    bottomLayout->addStretch();
    githubButton = new QPushButton();
    githubButton->setIcon(QIcon(QPixmap("../assets/github.png")));
    githubButton->setFixedSize(32, 32);
    bottomLayout->addWidget(githubButton);

    MoveCenter();
    LoadGallery();
}

MainWindow::~MainWindow()
{
}

void MainWindow::LoadGallery()
{
    for (int i = 0; i < 10; i++) {
        QListWidgetItem *item = new QListWidgetItem();
        item->setIcon(QIcon(QPixmap("../images/sample.jpeg")));
        galleryList->addItem(item);
    }
}

void MainWindow::MoveCenter()
{
    QRect desktopRect = QApplication::desktop()->availableGeometry(this);
    QPoint center = desktopRect.center();
    move(center.x() - width() * 0.5, center.y() - height() * 0.5);
}
