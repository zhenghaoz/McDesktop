#ifndef PARSER_H
#define PARSER_H

#include <QDomDocument>
#include <QDomElement>
#include <QJsonDocument>
#include <QJsonValue>

QJsonValue ParseDict(const QDomElement& element);
QJsonValue ParseArray(const QDomElement& element);
QJsonValue ParsePlist(const QDomElement& element);
QJsonDocument ParsePlist(const QDomDocument& doc);

#endif // PARSER_H
