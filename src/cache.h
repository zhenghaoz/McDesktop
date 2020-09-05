#ifndef CACHE_H
#define CACHE_H

#include <thread>
#include <QString>
#include <QImage>
#include <QVector>
#include <QPixmap>
#include <SolTrack.h>

struct CachedLocation
{
    double longitude;
    double latitude;
};

struct CachedFrame
{
    QPixmap thumb;
    QString path;
    double altitude;
    double azimuth;
    double GetDistance(const CachedLocation& location, const Time& tm) const;
};

struct CachedPicture
{
    QString name;
    QPixmap cover;
    CachedFrame lightFrame;
    CachedFrame darkFrame;
    QVector<CachedFrame> frames;

    CachedFrame GetFrame(const CachedLocation& location) const;
    CachedFrame GetFrame(const CachedLocation& location, const Time& tm) const;
};

class Cache
{
    static constexpr int kLocationCacheLease = 1;

    QString mHomePath;
    std::thread mSyncThread;
    std::thread pictureSyncThread;
    bool mIsTerminated = false;

    QString GetCacheDir() const;
    QString GetPictureDir() const;
    QVector<QString> ListPictures() const;
    QVector<QString> ListPictureCaches() const;
    void SyncPictureCache() const;
    void SyncLocationCache() const;
public:
    QVector<CachedPicture> ListCachedPictures() const;
    CachedLocation GetCachedLocation() const;
    Cache();
    ~Cache();
};

#endif // CACHE_H
