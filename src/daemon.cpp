#include "daemon.h"
#include "desktop.h"
#include "cache.h"

#include <QApplication>
#include <QMenu>
#include <QSystemTrayIcon>
#include <QtDebug>
#include <QTimer>

#include <spdlog/spdlog.h>

using namespace std;

Daemon::Daemon()
{    
    // Create actions
    QAction* settingAction = new QAction("Setting", this);
    connect(settingAction, &QAction::triggered, [this](){ mainWindow.show(); });
    QAction* exitAction = new QAction("Exit", this);
    connect(exitAction, &QAction::triggered, [](){ QApplication::quit(); });

    // Create menu
    QMenu* menu = new QMenu();
    menu->addAction(settingAction);
    menu->addAction(exitAction);

    // Create system tray
    QPixmap pixmap("../assets/icon.png");
    QSystemTrayIcon *trayIcon = new QSystemTrayIcon(QIcon(pixmap));
    trayIcon->setContextMenu(menu);
    trayIcon->setVisible(true);

    // Keeper timer
    DesktopKeeper();
    QTimer *keeperTimer = new QTimer(this);
    connect(keeperTimer, &QTimer::timeout, this, &Daemon::DesktopKeeper);
    keeperTimer->start(1000*60);
}

void Daemon::DesktopKeeper()
{
    const Cache& cache = Cache::getInstance();
    const CachedPicture& picture = cache.GetCurrentDesktop();
    const CachedFrame& frame = picture.GetFrame(cache.GetCachedLocation());
    spdlog::info("set wallpaper {}", frame.path.toStdString());
    setDesktopTask = async(launch::async, [=](){
        SetDesktop(frame.path);
    });
}
