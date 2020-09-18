// XML Parser - convert XML to JSON.
// Example:
// (XML)
//   <plist>
//     <array>
//       <dict>
//         <key>real</key>
//         <real>1.0</real>
//         <key>integer</key>
//         <integer>1</integer>
//       </dict>
//     </array>
//   </plist>
// (JSON)
// [{"real":1.0, "integer":1}]
#include "exception.h"
#include "parser.h"

#include <QJsonArray>
#include <QJsonObject>

using namespace std;

QJsonValue ParsePlist(const QDomElement& element);

QJsonValue ParseDict(const QDomElement& element)
{
    if (element.tagName() != "dict") {
        throw Exception(
                    Exception::ParseConfigurationError,
                    "expect dict, but recieve " + element.tagName().toStdString());
    }
    const QDomNodeList& children = element.childNodes();
    if (children.size() % 2) {
        throw Exception(
                    Exception::ParseConfigurationError,
                    "broken dict");
    }
    QJsonObject object;
    for (int i = 0; i < children.size(); i += 2) {
        const QDomElement& keyElement = children.at(i).toElement();
        if (keyElement.tagName() != "key") {
            throw Exception(
                        Exception::ParseConfigurationError,
                        "broken dict");
        }
        const QDomElement& valElement = children.at(i+1).toElement();
        const QString& key = keyElement.text();
        object.insert(key, ParsePlist(valElement));
    }
    return object;
}

QJsonValue ParseArray(const QDomElement& element)
{
    if (element.tagName() != "array") {
        throw Exception(
                    Exception::ParseConfigurationError,
                    "expect array, but recieve " + element.tagName().toStdString());
    }
    const QDomNodeList& children = element.childNodes();
    QJsonArray array;
    for (int i = 0; i < children.size(); i++) {
        const QDomNode& child = children.at(i);
        const QDomElement& element = child.toElement();
        array.append(ParsePlist(element));
    }
    return array;
}

QJsonValue ParseInt(const QDomElement& element)
{
    if (element.tagName() != "integer") {
        throw Exception(
                    Exception::ParseConfigurationError,
                    "expect integer, but recieve " + element.tagName().toStdString());
    }
    return stoi(element.text().toStdString());
}

QJsonValue ParseDouble(const QDomElement& element)
{
    if (element.tagName() != "real") {
        throw Exception(
                    Exception::ParseConfigurationError,
                    "expect real, but recieve " + element.tagName().toStdString());
    }
    return stod(element.text().toStdString());
}

QJsonValue ParsePlist(const QDomElement& element)
{
    const QString& tag = element.tagName();
    if (tag == "dict") {
        return ParseDict(element);
    } else if (tag == "array") {
        return ParseArray(element);
    } else if (tag == "real") {
        return ParseDouble(element);
    } else if (tag == "integer") {
        return ParseInt(element);
    }
    throw Exception(
                Exception::ParseConfigurationError,
                "unkown tag " + element.tagName().toStdString());
}

QJsonDocument ParsePlist(const QDomDocument& doc)
{
    const QDomElement& plistElement = doc.documentElement();
    if (plistElement.tagName() != "plist") {
        throw Exception(
                    Exception::ParseConfigurationError,
                    "expect plist, but recieve " + plistElement.tagName().toStdString());
    }
    const QJsonValue& val = ParsePlist(plistElement.firstChildElement());
    return QJsonDocument(val.toObject());
}
