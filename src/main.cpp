#include "mainwindow.h"
#include "daemon.h"

#include <QApplication>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QDebug>
#include <QObject>


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Daemon daemon;
    return a.exec();
}
