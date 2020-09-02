#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "cache.h"
#include "engine.h"

#include <QComboBox>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>

class MainWindow : public QWidget
{
    Q_OBJECT

    QComboBox *modeComboBox;
    QLabel *imageLabel, *nameLabel, *hintLabel;
    QListWidget *galleryList;
    QPushButton *addButton, *deleteButton, *githubButton;

    QStringList wallpapers;

    Engine engine;
    Cache cache;
    QVector<CachedPicture> pictures;
    int selected = 0;

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void LoadGallery();
    void MoveCenter();
    void AddWallpaper();
    void RemoveWallpaper();
    void OpenGitHub();
    void SelectPicture(QListWidgetItem* item);
};
#endif // MAINWINDOW_H
