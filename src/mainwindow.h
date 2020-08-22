#ifndef MAINWINDOW_H
#define MAINWINDOW_H

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

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void LoadGallery();
    void MoveCenter();
};
#endif // MAINWINDOW_H
