#ifndef CACHE_H
#define CACHE_H

#include <thread>
#include <QString>
#include <QImage>
#include <QVector>
#include <QPixmap>

struct CachedFrame
{
    QPixmap thumb;
    QString path;
    double altitude;
    double azimuth;
};

struct CachedPicture
{
    QString name;
    QPixmap cover;
    CachedFrame lightFrame;
    CachedFrame darkFrame;
    QVector<CachedFrame> frames;
};

class Cache
{
    QString mHomePath;
    std::thread mSyncThread;
    bool mIsTerminated = false;

    QString GetCacheLocation() const;
    QString GetPictureLocation() const;
    QVector<QString> ListPictures() const;
    QVector<QString> ListCaches() const;
    void Sync() const;
public:
    QVector<CachedPicture> ListCachedPictures() const;
    Cache();
    ~Cache();
};

#endif // CACHE_H
