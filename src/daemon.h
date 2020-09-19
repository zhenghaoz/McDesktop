#ifndef DAEMON_H
#define DAEMON_H

#include "mainwindow.h"

#include <QObject>
#include <QSystemTrayIcon>

#include <future>

class Daemon : public QObject
{
    MainWindow mainWindow;
    QSystemTrayIcon *trayIcon;
    std::future<void> setDesktopTask;
public:
    Daemon();
    void DesktopKeeper();
};

#endif // DAEMON_H
