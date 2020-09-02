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
    rootDir.mkpath(GetCacheLocation());
    rootDir.mkpath(GetPictureLocation());

    // Create sync thread
    mSyncThread = thread(&Cache::Sync, this);
}

Cache::~Cache()
{
    mIsTerminated = true;
    mSyncThread.join();
}

void Cache::Sync() const
{
    while (!mIsTerminated) {
        // List caches
        const QVector<QString> caches = ListCaches();
        QSet<QString> cacheSet(caches.begin(), caches.end());

        // List pictures
        const QVector<QString> pictures = ListPictures();
        for (const QString& picture : pictures) {
            const QString& path = GetPictureLocation() + "/" + picture;
            const QString& checksum = Checksum(path);
            if (!cacheSet.contains(checksum)) {
                spdlog::info("add cache for {}", path.toStdString());
                Heic heic = Heic::Load(path);
                const QString cachePath = GetCacheLocation() + "/" + checksum;
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
    spdlog::info("sync thread exit");
}

QVector<QString> Cache::ListPictures() const
{
    const QDir& pictureDir(GetPictureLocation());
    return pictureDir.entryList(QStringList() << "*.heic" << "*.HEIC", QDir::Files).toVector();
}

QVector<QString> Cache::ListCaches() const
{
    const QDir& pictureDir(GetCacheLocation());
    return pictureDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot).toVector();
}

QString Cache::GetCacheLocation() const
{
    return mHomePath + "/ddesktop/cache";
}

QString Cache::GetPictureLocation() const
{
    return mHomePath + "/ddesktop/pictures";
}

QVector<CachedPicture> Cache::ListCachedPictures() const
{
    QVector<CachedPicture> pictures;
    const QVector<QString>& caches = ListCaches();
    for (const QString& cache : caches) {
        const QString& path = GetCacheLocation() + "/" + cache;
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
            frame.thumb = QPixmap(path + "thumb_" + QString::number(index) + ".png");
            if (l == index) picture.lightFrame = frame;
            if (d == index) picture.darkFrame = frame;
            picture.frames.push_back(frame);
        }
        pictures.push_back(picture);
    }
    return pictures;
}
