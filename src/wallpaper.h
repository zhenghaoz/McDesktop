#ifndef WALLPAPER_H
#define WALLPAPER_H

#include <QImage>

struct Heic
{
    std::vector<QImage> images;

    static Heic Load(const std::string& path);
};

#endif // WALLPAPER_H
