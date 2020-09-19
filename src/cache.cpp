// Cache - designed for:
// 1. Update location by IP.
// 2. Update wallpaper cache.
#include "cache.h"
#include "exception.h"
#include "heic.h"

#include <QDir>
#include <QSet>
#include <QDirIterator>
#include <QStandardPaths>
#include <QCryptographicHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSettings>

#include <httplib.h>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <cmath>
#include <iostream>

using namespace std;

QString Checksum(const QString &fileName)
{
    QFile f(fileName);
    if (f.open(QFile::ReadOnly)) {
        QCryptographicHash hash(QCryptographicHash::Md5);
        if (hash.addData(&f)) {
            return hash.result().toHex();
        }
    }
    throw Exception(
                Exception::OpenFileError,
                "can't open file " + fileName.toStdString());
}

CachedLocation GetLocationFromIP()
{
    httplib::Client cli("http://ip-api.com");
    if (auto res = cli.Get("/json")) {
        const string json = res->body;
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(QByteArray::fromStdString(json), &error);
        if (!doc.isNull() && error.error == QJsonParseError::NoError) {
            QJsonObject obj = doc.object();
            if (obj.contains("lat") && obj.contains("lon")) {
                CachedLocation location;
                location.latitude = obj.value("lat").toDouble();
                location.longitude = obj.value("lon").toDouble();
                return location;
            }
        }
        throw Exception(
                    Exception::ParseJSONError,
                    "failed to get location from IP");
    }
    throw Exception(
                Exception::NetworkError,
                "failed to get location from IP");
}

Position GetSolarPosition(double lat, double lon, const Time& tm)
{
    // Create location
    Location loc;
    loc.longitude = lon;
    loc.latitude = lat;
    loc.pressure = 101.325;
    loc.temperature = 288;

    // Compute positions
    Position pos;
    SolTrack(tm, loc, &pos, 1, 1, 0, 0);
    return pos;
}

double CachedFrame::GetDistance(const CachedLocation& location, const Time& tm) const
{
    const Position& position = GetSolarPosition(location.latitude, location.longitude, tm);
    double distance = acos(
                cos(position.altitude*PI/180) * cos(altitude*PI/180) * cos(azimuth*PI/180 - position.azimuthRefract*PI/180)
                + sin(position.altitude*PI/180) * sin(altitude*PI/180));
    spdlog::info("distance between ({},{}) and ({},{}) is {}",
                 position.azimuthRefract, position.altitudeRefract,
                 azimuth, altitude, distance);
    return distance;
}

CachedFrame CachedPicture::GetFrame(const CachedLocation& location) const
{
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
    return GetFrame(location, time);
}

CachedFrame CachedPicture::GetFrame(const CachedLocation& location, const Time& tm) const
{
    return *min_element(frames.begin(), frames.end(), [&](const CachedFrame& lhs, const CachedFrame& rhs)->bool{
        return lhs.GetDistance(location, tm) < rhs.GetDistance(location, tm);
    });
}

Cache::Cache()
{
    // Find home path
    const QStringList& homePaths = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
    if (homePaths.empty()) {
        spdlog::critical("home directory not found");
        exit(-1);
    }
    homePath = homePaths.front();
    spdlog::info("find home directory: {}", homePath.toStdString());

    // Create directories if not exist
    const QDir& rootDir = QDir::root();
    rootDir.mkpath(GetCacheDir());
    rootDir.mkpath(GetPictureDir());

    // Create sync thread
    pictureSyncThread = thread(&Cache::SyncPictureCache, this);
    locationSyncThread = thread(&Cache::SyncLocationCache, this);
}

Cache::~Cache()
{
    isTerminated = true;
    NotifyLocationSyncer();
    NotifyCacheSyncer();
    pictureSyncThread.join();
    locationSyncThread.join();
}

void Cache::SyncPictureCache()
{
    while (!isTerminated) {
        bool changed = false;

        // List caches
        const QVector<QString> caches = ListCaches();
        QSet<QString> cacheSet;
        for (const QString& cache : caches) {
            cacheSet.insert(cache);
        }

        // List pictures
        const QVector<QString> pictures = ListPictures();
        for (const QString& picture : pictures) {
            const QString& path = GetPictureDir() + "/" + picture;
            const QString& checksum = Checksum(path);
            if (!cacheSet.contains(checksum)) {
                spdlog::info("add cache for {}", path.toStdString());
                Heic heic = Heic::Load(path);
                const QString cachePath = GetCacheDir() + "/" + checksum;
                QDir(cachePath).mkpath(".");
                heic.Save(cachePath);
                changed = true;
            } else {
                cacheSet.remove(checksum);
            }
        }

        // Remove orphan
        for (const QString& cache : cacheSet) {
            spdlog::info("orphan {}", cache.toStdString());
            QDir dir(GetCacheDir() + "/" + cache);
            if (dir.removeRecursively()) {
                spdlog::info("remove orphan sucess");
                changed = true;
            } else {
                spdlog::info("remove orphan failed");
            }
        }

        if (changed && pictureSyncCallback != nullptr) {
            pictureSyncCallback();
        }

        // Sleep
        {
            chrono::seconds timespan(10);
            unique_lock<mutex> lk(pictureSyncMutex);
            pictureSyncCond.wait_for(lk, timespan);
        }
    }
    spdlog::info("picture cache sync thread exit");
}

void Cache::SyncLocationCache()
{
    while (!isTerminated) {
        CachedLocation location = GetLocationFromIP();
        spdlog::info("get location from IP, lon = {}, lat = {}", location.longitude, location.latitude);
        QSettings settings;
        settings.setValue("longitude", location.longitude);
        settings.setValue("latitude", location.latitude);

        // Sleep
        {
            chrono::hours timespan(kLocationCacheLease);
            unique_lock<mutex> lk(locationSyncMutex);
            locationSyncCond.wait_for(lk, timespan);
        }
    }
    spdlog::info("location cache sync thread exit");
}

QVector<QString> Cache::ListPictures() const
{
    const QDir& pictureDir(GetPictureDir());
    return pictureDir.entryList(QStringList() << "*.heic" << "*.HEIC", QDir::Files).toVector();
}

QVector<QString> Cache::ListCaches() const
{
    const QDir& pictureDir(GetCacheDir());
    return pictureDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot).toVector();
}

QString Cache::GetCacheDir() const
{
    return homePath + "/ddesktop/cache";
}

QString Cache::GetPictureDir() const
{
    return homePath + "/ddesktop/pictures";
}

// Get latest pictures from cache.
QVector<CachedPicture> Cache::GetCachedPictures() const
{
    QVector<CachedPicture> pictures;
    const QVector<QString>& caches = ListCaches();
    for (const QString& cache : caches) {
        const QString& path = GetCacheDir() + "/" + cache;
        CachedPicture picture;

        // Load cover
        picture.cover = QPixmap(path + "/cover.jpg");

        // Load config
        QFile configFile(path + "/config.json");
        if (!configFile.open(QFile::ReadOnly)) {
            throw Exception(
                        Exception::OpenFileError,
                        "can't open file " + (path + "/config.json").toStdString());
        }
        const QJsonDocument& configDoc = QJsonDocument::fromJson(configFile.readAll());
        const QJsonObject& rootObject = configDoc.object();
        // Load name
        picture.name = rootObject.value("name").toString();
        // Load dark and light frame
        const QJsonObject& apObject = rootObject.value("ap").toObject();
        int l = apObject.value("l").toInt();
        int d = apObject.value("d").toInt();
        // Load frames
        const QJsonArray& siArray = rootObject.value("si").toArray();
        for (int i = 0; i < siArray.size(); i++) {
            const QJsonObject& frameObject = siArray.at(i).toObject();
            CachedFrame frame;
            int index = frameObject.value("i").toInt();
            frame.azimuth = frameObject.value("z").toDouble();
            frame.altitude = frameObject.value("a").toDouble();
            frame.path = path + '/' + QString::number(index) + ".jpg";
            frame.thumb = QPixmap(path + "/thumb_" + QString::number(index) + ".jpg");
            if (l == index) picture.lightFrame = frame;
            if (d == index) picture.darkFrame = frame;
            picture.frames.push_back(frame);
        }
        pictures.push_back(picture);
    }
    return pictures;
}

// Get latest location from cache.
CachedLocation Cache::GetCachedLocation() const
{
    QSettings settings;
    CachedLocation location;
    location.longitude = settings.value("longitude", 120.94).toDouble();
    location.latitude = settings.value("latitude", 28.14).toDouble();
    spdlog::info("get cached location, lon = {}, lat = {}", location.longitude, location.latitude);
    return location;
}

// Notify location cache syncer to wake up.
void Cache::NotifyLocationSyncer()
{
    locationSyncCond.notify_all();
}

// Notify picture cache syncer to wake up.
void Cache::NotifyCacheSyncer()
{
    pictureSyncCond.notify_all();
}

// Set current desktop
void Cache::SetCurrentDesktop(const QString& name)
{
    spdlog::info("set current desktop {}", name.toStdString());
    // Validate
    const auto& pictures = GetCachedPictures();
    if (!any_of(pictures.begin(), pictures.end(), [&](const CachedPicture& picture){ return picture.name == name; })) {
        throw Exception(Exception::PictureNotExistsError, "picture not exists");
    }
    // Save
    QSettings settings;
    settings.setValue("wallpaper", name);
    // Notify
    {
        lock_guard<mutex> lock(desktopChangeCallbackMtx);
        if (desktopChangeCallback) {
            desktopChangeCallback();
        }
    }
}

// Get current desktop
std::optional<CachedPicture> Cache::GetCurrentDesktop() const
{
    spdlog::info("get current desktop");
    // Load
    QSettings settings;
    const auto& name = settings.value("wallpaper", "").toString();
    if (name.isEmpty()) {
        return nullopt;
    }
    // Fetch
    const auto& pictures = GetCachedPictures();
    for (const auto& picture : pictures) {
        if (picture.name == name) {
            spdlog::info("{} {}", name.toStdString(), picture.name.toStdString());
            return picture;
        }
    }
    throw Exception(Exception::PictureNotExistsError, "picture " + name.toStdString() + " not exists");
}


// Notify picture cache syncer to wake up and call pictureSyncCallback if something happened.
void Cache::ListenOnCacheChange(std::function<void(void)> action)
{
    lock_guard<mutex> lock(callbackMutex);
    this->pictureSyncCallback = action;
}

void Cache::ListenOnDesktopChange(std::function<void(void)> desktopChangeCallback)
{
    lock_guard<mutex> lock(desktopChangeCallbackMtx);
    this->desktopChangeCallback = desktopChangeCallback;
}
