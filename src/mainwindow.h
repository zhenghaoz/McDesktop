#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "cache.h"

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

    Cache cache;
    QVector<CachedPicture> pictures;
    int selected = -1;
    int play = 0;

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void LoadGallery();
    void MoveCenter();
    void AddWallpaper();
    void RemoveWallpaper();
    void OpenGitHub();
    void SelectPicture(QListWidgetItem* item);
    void PlayPreview();
};
#endif // MAINWINDOW_H
