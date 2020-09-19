#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <string>

class Exception
{
public:
    Exception();
    Exception(int errorCode, const std::string& message): message(message) {}
    std::string what() const { return message; }

    std::string message;

    enum {
        ParseConfigurationError,
        ParseHEICError,
        OpenFileError,
        NetworkError,
        ParseJSONError,
        PictureNotExistsError,
    };

};

#endif // EXCEPTION_H
