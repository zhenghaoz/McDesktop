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
#ifndef PARSER_H
#define PARSER_H

#include <QDomDocument>
#include <QDomElement>
#include <QJsonDocument>
#include <QJsonValue>

QJsonDocument ParsePlist(const QDomDocument& doc);

#endif // PARSER_H
