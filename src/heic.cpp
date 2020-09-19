// HEIC Reader - fetch data from HEIC file.
#include "exception.h"
#include "heic.h"
#include "parser.h"

#include <QByteArray>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPainter>
#include <QPixmap>
#include <QtXml/QDomDocument>

#include <boost/any.hpp>
#include <libheif/heif_cxx.h>
#include <Plist.hpp>
#include <spdlog/spdlog.h>

using namespace std;
using namespace heif;
using namespace Plist;

QImage Crop(const QImage& image, int width, int height)
{
    // Step 1: Scale image
    double ratio = static_cast<double>(height) / width;
    double imgRatio = static_cast<double>(image.height()) / image.width();
    int scaleHeight = height;
    int scaleWidth = width;
    if (imgRatio < ratio) {
        // +-+------+-+
        // | |      | |
        // +-+------+-+
        scaleWidth = scaleHeight / imgRatio;
    } else if (imgRatio > ratio) {
        // +------+
        // +------+
        // |      |
        // +------+
        // +------+
        scaleHeight = scaleWidth * imgRatio;
    }
    const QImage& scaledImage = image.scaled(scaleWidth, scaleHeight, Qt::KeepAspectRatio);
    // Step 2: Crop image
    int left = (static_cast<double>(scaleWidth) - width) / 2;
    int top = (static_cast<double>(scaleHeight) - height) / 2;
    QRect rect(left, top, width, height);
    return scaledImage.copy(rect);
}

Heic Heic::Load(const QString& path)
{
    Heic heic;
    heic.name = path.toStdString();

    // Read HEIC file
    Context context;
    context.read_from_file(path.toStdString());

    // Load images
    const vector<heif_item_id>& imageIds = context.get_list_of_top_level_image_IDs();
    for (const heif_item_id& imageId : imageIds) {

        // Fetch image
        ImageHandle handle = context.get_image_handle(imageId);
        Image img = handle.decode_image(heif_colorspace_RGB, heif_chroma_interleaved_RGB);
        int stride;
        const uint8_t* data = img.get_plane(heif_channel_interleaved, &stride);

        // Decode image
        int height = img.get_height(heif_channel_interleaved);
        int width = img.get_width(heif_channel_interleaved);
        heic.images.push_back(QImage(data, width, height, stride, QImage::Format_RGB888).copy());

        // Fetch metadata
        const vector<heif_item_id>& metaIds = handle.get_list_of_metadata_block_IDs();
        if (metaIds.size() == 1
                && handle.get_metadata_type(metaIds.front()) == "mime"
                && handle.get_metadata_content_type(metaIds.front()) == "application/rdf+xml") {

            if (!heic.config.empty()) {
                throw Exception(
                            Exception::ParseHEICError,
                            "duplicate metadata");
            }

            // Parse metadata
            const vector<uint8_t>& metadata = handle.get_metadata(metaIds.front());
            string metaText(metadata.begin(), metadata.end());
            QDomDocument metaDoc;
            metaDoc.setContent(QString::fromStdString(metaText));
            const QDomElement& rootElement = metaDoc.documentElement();
            const QDomElement& rdfElement = rootElement.firstChildElement();
            const QDomElement& descElement = rdfElement.firstChildElement();
            const QString& solarConfig = descElement.attribute("apple_desktop:solar");

            // Parse plist
            const string& plistText = QByteArray::fromBase64(solarConfig.toUtf8()).toStdString();
            boost::any message;
            Plist::readPlist(plistText.c_str(), plistText.size(), message);
            vector<char> buf;
            Plist::writePlistXML(buf, message);
            heic.config = string(buf.begin(), buf.end());
        }
    }

    return heic;
}

void Heic::Save(const QString &path) const
{
    spdlog::info("save cache of {} to {}", name, path.toStdString());

    // Parse config
    QDomDocument dom;
    dom.setContent(QString::fromStdString(config));
    QJsonDocument json = ParsePlist(dom);
    const QJsonObject& ap = json.object().value("ap").toObject();
    size_t lightFrameId = ap.value("l").toInt();
    size_t darkFrameId = ap.value("d").toInt();

    // Save name
    const QFileInfo& heicFile(QString::fromStdString(name));
    const QStringList& nameAndExt = heicFile.fileName().split(".");
    const QString& name = nameAndExt.at(0);
    spdlog::info("\tname: {}", name.toStdString());
    QJsonObject object = json.object();
    object.insert("name", name);
    json.setObject(object);

    // Save config
    QFile configFile(path + "/config.json");
    configFile.open(QIODevice::WriteOnly);
    configFile.write(json.toJson());
    configFile.close();

    QImage darkFrame, lightFrame;
    spdlog::info("\timages: {}", images.size());
    for (size_t i = 0; i < images.size(); i++) {
        const QImage& image = images[i];
        const QString& fileName = path + "/" + QString::number(i) + ".jpg";
        image.save(fileName);
        // Generate thumbnails
        const QString& thumbName = path + "/thumb_" + QString::number(i) + ".jpg";
        const QImage& thumb = Crop(image, kThumbWidth, kThumbHeight);
        thumb.save(thumbName);
        if (i == darkFrameId) {
            darkFrame = thumb.copy();
        }
        if (i == lightFrameId) {
            lightFrame = thumb.copy();
        }
    }

    // Generate cover
    const auto thumbWidth = darkFrame.width();
    const auto thumbHeight = lightFrame.height();
    const QString& coverName = path + "/cover.jpg";
    QPixmap coverPixmap(thumbWidth, thumbHeight);
    QPainter painter(&coverPixmap);
    QRegion r1(QRect(0, 0, thumbWidth/2, thumbHeight));
    painter.setClipRegion(r1);
    painter.drawImage(0, 0, lightFrame);
    QRegion r2(QRect(thumbWidth/2, 0, thumbWidth/2, thumbHeight));
    painter.setClipRegion(r2);
    painter.drawImage(0, 0, darkFrame);
    coverPixmap.save(coverName);
}
