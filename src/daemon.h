#ifndef DAEMON_H
#define DAEMON_H

#include "mainwindow.h"

#include <QObject>

#include <future>

class Daemon : public QObject
{
    MainWindow mainWindow;
    std::future<void> setDesktopTask;
public:
    Daemon();
    void DesktopKeeper();
};

#endif // DAEMON_H
