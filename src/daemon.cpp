#include "daemon.h"
#include "exception.h"
#include "desktop.h"
#include "cache.h"

#include <QApplication>
#include <QMenu>
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
    trayIcon = new QSystemTrayIcon(QIcon(pixmap));
    trayIcon->setContextMenu(menu);
    trayIcon->setVisible(true);

    // Keeper timer
    DesktopKeeper();
    QTimer *keeperTimer = new QTimer(this);
    connect(keeperTimer, &QTimer::timeout, this, &Daemon::DesktopKeeper);
    keeperTimer->start(1000*60*10);

    // Register callback
    Cache& cache = Cache::getInstance();
    cache.ListenOnDesktopChange([this](){ DesktopKeeper(); });
}

void Daemon::DesktopKeeper()
{
    Cache& cache = Cache::getInstance();
    try {
        const optional<CachedPicture>& picture = cache.GetCurrentDesktop();
        if (picture.has_value()) {
            const CachedFrame& frame = picture.value().GetFrame(cache.GetCachedLocation());
            spdlog::info("set wallpaper {}", frame.path.toStdString());
            setDesktopTask = async(launch::async, [=](){
                SetDesktop(frame.path);
            });
        }
    } catch (const Exception& e) {
        trayIcon->showMessage("Exception", QString::fromStdString(e.what()), QSystemTrayIcon::Critical);
    }
}
