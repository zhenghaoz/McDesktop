#ifndef ENGINE_H
#define ENGINE_H

#include <thread>

class Engine
{
    bool isTerminated = false;
    std::thread daemon;

    void MainLoop();
    int GetLocationFromIP(double &lat, double &lon);
    int GetSolarPosition(double lat, double lon, double &altitude, double &azimuth);

public:
    Engine();
    ~Engine();

    static constexpr int Ok = 0;
    static constexpr int HttpError = 1;
    static constexpr int JSONParseError = 2;
};

#endif // ENGINE_H
