#include "engine.h"

#include <iostream>
#include <httplib.h>
#include <QJsonDocument>
#include <QJsonObject>
#include <SolTrack.h>
#include <stdlib.h>

using namespace std;

Engine::Engine()
{
    daemon = thread(&Engine::MainLoop, this);
}

Engine::~Engine()
{
    isTerminated = true;
    daemon.join();
}

void Engine::MainLoop()
{
    while (!isTerminated) {
        chrono::seconds timespan(10);
        this_thread::sleep_for(timespan);

        double lat = 0, lon = 0;
        GetLocationFromIP(lat, lon);


    }
}

int Engine::GetLocationFromIP(double &lat, double &lon)
{
    httplib::Client cli("http://ip-api.com");
    if (auto res = cli.Get("/json")) {
        const string json = res->body;
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(QByteArray::fromStdString(json), &error);
        if (!doc.isNull() && error.error == QJsonParseError::NoError) {
            QJsonObject obj = doc.object();
            if (obj.contains("lat") && obj.contains("lon")) {
                lat = obj.value("lat").toDouble();
                lon = obj.value("lon").toDouble();
                return Ok;
            }
        }
        return JSONParseError;
    }
    return HttpError;
}

int Engine::GetSolarPosition(double lat, double lon, double &altitude, double &azimuth)
{
    // Create time
    auto currentTime = chrono::system_clock::now();
    time_t tt = chrono::system_clock::to_time_t(currentTime);
    tm local_tm = *gmtime(&tt);
    Time time;
    time.year = local_tm.tm_year + 1900;
    time.month = local_tm.tm_mon + 1;
    time.day = local_tm.tm_mday;
    time.hour = local_tm.tm_hour;
    time.minute = local_tm.tm_min;
    time.second = local_tm.tm_sec;

    // Create location
    Location loc;
    loc.longitude = lon;
    loc.latitude = lat;
    loc.pressure = 101.325;
    loc.temperature = 288;

    // Compute positions
    Position pos;
    SolTrack(time, loc, &pos, 1, 1, 0, 0);
    altitude = pos.azimuthRefract;
    azimuth = pos.altitudeRefract;
    return Ok;
}
