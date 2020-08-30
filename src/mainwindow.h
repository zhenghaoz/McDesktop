#ifndef MAINWINDOW_H
#define MAINWINDOW_H

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

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void LoadGallery();
    void MoveCenter();
    void AddWallpaper();
    void RemoveWallpaper();
    void OpenGitHub();
};
#endif // MAINWINDOW_H
