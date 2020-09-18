// Cache - designed for:
// 1. Update location by IP.
// 2. Update wallpaper cache.
#ifndef CACHE_H
#define CACHE_H

#include <QString>
#include <QImage>
#include <QVector>
#include <QPixmap>

#include <SolTrack.h>

#include <condition_variable>
#include <functional>
#include <thread>
#include <mutex>

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
    static constexpr int kPictureCacheLease = 5;

    QString homePath;
    QVector<CachedPicture> pictures;
    std::function<void(void)> pictureSyncCallback;

    bool isTerminated = false;
    std::mutex pictureSyncMutex;
    std::mutex locationSyncMutex;
    std::condition_variable pictureSyncCond;
    std::condition_variable locationSyncCond;
    std::thread pictureSyncThread;
    std::thread locationSyncThread;

    QString GetCacheDir() const;
    QString GetPictureDir() const;
    QVector<QString> ListPictures() const;
    QVector<QString> ListPictureCaches() const;
    void SyncPictureCache();
    void SyncLocationCache();

    Cache();
    ~Cache();
    Cache(const Cache& cache) = delete;
    Cache(Cache&& cache) = delete;

public:

    static Cache& getInstance(){
      static Cache instance;
      // volatile int dummy{};
      return instance;
    }

    QVector<CachedPicture> ListCachedPictures() const;
    CachedLocation GetCachedLocation() const;
    void SetWallpaper(const QString& name);
    CachedPicture GetCurrentDesktop() const;
    void NotifyLocationSyncer();
    void NotifyPictureSyncer();
    void NotifyPictureSyncer(std::function<void(void)> pictureSyncCallback);

};

#endif // CACHE_H
