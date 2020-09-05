#include "cache.h"
#include "heic.h"
#include "exception.h"
#include <spdlog/spdlog.h>
#include <iostream>
#include <QDir>
#include <QSet>
#include <QDirIterator>
#include <QStandardPaths>
#include <QCryptographicHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSettings>
#include <cmath>

#include <algorithm>
#include <httplib.h>

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
    throw SunDesktopException(
                SunDesktopException::OpenFileError,
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
        throw SunDesktopException(
                    SunDesktopException::ParseJSONError,
                    "failed to get location from IP");
    }
    throw SunDesktopException(
                SunDesktopException::NetworkError,
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
    mHomePath = homePaths.front();
    spdlog::info("find home directory: {}", mHomePath.toStdString());

    // Create directories if not exist
    const QDir& rootDir = QDir::root();
    rootDir.mkpath(GetCacheDir());
    rootDir.mkpath(GetPictureDir());

    // Create sync thread
    mSyncThread = thread(&Cache::SyncPictureCache, this);
    pictureSyncThread = thread(&Cache::SyncLocationCache, this);
}

Cache::~Cache()
{
    mIsTerminated = true;
    mSyncThread.join();
    pictureSyncThread.join();
}

void Cache::SyncPictureCache() const
{
    while (!mIsTerminated) {
        // List caches
        const QVector<QString> caches = ListPictureCaches();
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
            } else {
                cacheSet.remove(checksum);
            }
        }

        spdlog::info("looping");
        chrono::seconds timespan(10);
        this_thread::sleep_for(timespan);
    }
    spdlog::info("picture cache sync thread exit");
}

void Cache::SyncLocationCache() const
{
    while (!mIsTerminated) {
        CachedLocation location = GetLocationFromIP();
        spdlog::info("get location from IP, lon = {}, lat = {}", location.longitude, location.latitude);
        QSettings settings;
        settings.setValue("longitude", location.longitude);
        settings.setValue("latitude", location.latitude);
        chrono::hours timespan(kLocationCacheLease);
        this_thread::sleep_for(timespan);
    }
    spdlog::info("location cache sync thread exit");
}

QVector<QString> Cache::ListPictures() const
{
    const QDir& pictureDir(GetPictureDir());
    return pictureDir.entryList(QStringList() << "*.heic" << "*.HEIC", QDir::Files).toVector();
}

QVector<QString> Cache::ListPictureCaches() const
{
    const QDir& pictureDir(GetCacheDir());
    return pictureDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot).toVector();
}

QString Cache::GetCacheDir() const
{
    return mHomePath + "/ddesktop/cache";
}

QString Cache::GetPictureDir() const
{
    return mHomePath + "/ddesktop/pictures";
}

QVector<CachedPicture> Cache::ListCachedPictures() const
{
    QVector<CachedPicture> pictures;
    const QVector<QString>& caches = ListPictureCaches();
    for (const QString& cache : caches) {
        const QString& path = GetCacheDir() + "/" + cache;
        CachedPicture picture;

        // Load cover
        picture.cover = QPixmap(path + "/cover.png");

        // Load config
        QFile configFile(path + "/config.json");
        if (!configFile.open(QFile::ReadOnly)) {
            throw SunDesktopException(
                        SunDesktopException::OpenFileError,
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
            frame.path = path + QString::number(index) + ".png";
            frame.thumb = QPixmap(path + "/thumb_" + QString::number(index) + ".png");
            if (l == index) picture.lightFrame = frame;
            if (d == index) picture.darkFrame = frame;
            picture.frames.push_back(frame);
        }
        pictures.push_back(picture);
    }
    return pictures;
}

CachedLocation Cache::GetCachedLocation() const
{
    QSettings settings;
    CachedLocation location;
    location.longitude = settings.value("longitude", 120.94).toDouble();
    location.latitude = settings.value("latitude", 28.14).toDouble();
    spdlog::info("get cached location, lon = {}, lat = {}", location.longitude, location.latitude);
    return location;
}
