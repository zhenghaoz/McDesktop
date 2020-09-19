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

#include <atomic>
#include <condition_variable>
#include <functional>
#include <thread>
#include <mutex>
#include <optional>

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

    std::function<void(void)> pictureSyncCallback;

    std::atomic<bool> isTerminated = false;

    std::function<void(void)> desktopChangeCallback;
    std::mutex desktopChangeCallbackMtx;

    // Cached pictures
    std::atomic<int> writeVersion = 1;
    std::atomic<int> readVersion = 0;
    std::mutex pictureMutex;

    std::mutex callbackMutex;
    std::mutex pictureSyncMutex;
    std::mutex locationSyncMutex;
    std::condition_variable pictureSyncCond;
    std::condition_variable locationSyncCond;
    std::thread pictureSyncThread;
    std::thread locationSyncThread;

    QString GetCacheDir() const;

    QVector<QString> ListPictures() const;
    QVector<QString> ListCaches() const;
    void SyncPictureCache();
    void SyncLocationCache();

    Cache();
    ~Cache();
    Cache(const Cache& cache) = delete;
    Cache(Cache&& cache) = delete;

public:

    static Cache& getInstance()
    {
        // TODO: Is it thread-safe?
        static Cache instance;
        return instance;
    }

    QString GetHomeDir() const
    {
        return homePath;
    }

        QString GetPictureDir() const;

    // Get latest pictures from cache.
    QVector<CachedPicture> GetCachedPictures() const;

    // Get latest location from cache.
    CachedLocation GetCachedLocation() const;

    // Set current desktop
    void SetCurrentDesktop(const QString& name);

    // Get current desktop
    std::optional<CachedPicture> GetCurrentDesktop() const;

    // Notify location cache syncer to wake up.
    void NotifyLocationSyncer();

    // Notify picture cache syncer to wake up.
    void NotifyCacheSyncer();

    void ListenOnCacheChange(std::function<void(void)> cacheChangeCallback);

    void ListenOnDesktopChange(std::function<void(void)> pictureChangeCallback);

};

#endif // CACHE_H
