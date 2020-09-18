#include "desktop.h"

#include <QProcess>

#include <spdlog/spdlog.h>

void SetDesktop(const QString& path)
{

#ifdef __linux__
    const QString& command = "dbus-send "
                             "--dest=com.deepin.daemon.Appearance /com/deepin/daemon/Appearance "
                             "--print-reply com.deepin.daemon.Appearance.SetMonitorBackground "
                             "string:\"eDP-1\" string:\"file://" + path + "\"";
#else
#error("unsupported platform")
#endif

    spdlog::info("execute {}", command.toStdString());
    QProcess process;
    process.execute(command);
}
