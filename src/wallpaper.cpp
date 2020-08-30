#include "wallpaper.h"

#include <algorithm>  // std::min_element
#include <fstream>
#include <iostream>
#include <map>
#include <QtXml/QDomDocument>
#include <libheif/heif_cxx.h>
#include <Plist.hpp>
#include <QByteArray>
#include <boost/any.hpp>

using namespace std;
using namespace heif;
using namespace Plist;

Heic Heic::Load(const std::string& path)
{
    Heic heic;

    // Read HEIC file
    Context context;
    context.read_from_file(path);

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
            // Parse metadata
            const vector<uint8_t> metadata = handle.get_metadata(metaIds.front());
            string text(metadata.begin(), metadata.end());
            cout << text << endl;
            QDomDocument doc;
            doc.setContent(QString::fromStdString(text));
            QDomElement root = doc.documentElement();
            QDomElement rdf = root.firstChildElement();
            QDomElement description = rdf.firstChildElement();
            QString solar = description.attribute("apple_desktop:solar");
            string plist = QByteArray::fromBase64(solar.toUtf8()).toStdString();
            boost::any message;
            Plist::readPlist(plist.c_str(), plist.size(), message);
            vector<char> a;
            Plist::writePlistXML(a, message);
            cout << string(a.begin(), a.end()) << endl;
            break;
        }
    }

    return heic;
}
