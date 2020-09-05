#include "heic.h"

#include "exception.h"
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
                throw SunDesktopException(
                            SunDesktopException::ParseHEICError,
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
    // Parse config
    QDomDocument dom;
    dom.setContent(QString::fromStdString(config));
    QJsonDocument json = ParsePlist(dom);
    const QJsonObject& ap = json.object().value("ap").toObject();
    size_t lightFrameId = ap.value("l").toInt();
    size_t darkFrameId = ap.value("d").toInt();

    // Save name
    const QFileInfo& heicFile(QString::fromStdString(name));
    const QString& name = heicFile.fileName().split(".").at(0);
    QJsonObject object = json.object();
    object.insert("name", name);
    json.setObject(object);

    // Save config
    QFile configFile(path + "/config.json");
    configFile.open(QIODevice::WriteOnly);
    configFile.write(json.toJson());
    configFile.close();

    QImage darkFrame, lightFrame;
    for (size_t i = 0; i < images.size(); i++) {
        const QImage& image = images[i];
        const QString& fileName = path + "/" + QString::number(i) + ".png";
        image.save(fileName);
        // Generate thumbnails
        const QString& thumbName = path + "/thumb_" + QString::number(i) + ".png";
        const QImage& thumb = image.scaled(kThumbWidth, kThumbHeight, Qt::KeepAspectRatio);
        thumb.save(thumbName);
        if (i == darkFrameId) {
            darkFrame = thumb.copy();
        }
        if (i == lightFrameId) {
            lightFrame = thumb.copy();
        }
    }

    // Generate cover
    const QString& coverName = path + "/cover.png";
    QPixmap coverPixmap(kThumbWidth, kThumbHeight);
    QPainter painter(&coverPixmap);
    QRegion r1(QRect(0, 0, kThumbWidth/2, kThumbHeight));
    painter.setClipRegion(r1);
    painter.drawImage(0, 0, lightFrame);
    QRegion r2(QRect(kThumbWidth/2, 0, kThumbWidth, kThumbHeight));
    painter.setClipRegion(r2);
    painter.drawImage(0, 0, darkFrame);
    coverPixmap.save(coverName);
}