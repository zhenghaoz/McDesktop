// HEIC Reader - fetch data from HEIC file.
#ifndef WALLPAPER_H
#define WALLPAPER_H

#include <QImage>
#include <QString>

struct Heic
{
    std::vector<QImage> images;
    std::string config;
    std::string name;

    void Save(const QString& path) const;

    static Heic Load(const QString& fileName);

    static constexpr int kThumbWidth = 480;     // the width of thumbnails
    static constexpr int kThumbHeight = 270;    // the height of thumbnails
};

#endif // WALLPAPER_H
