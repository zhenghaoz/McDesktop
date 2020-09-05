#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <string>

class SunDesktopException
{
public:
    SunDesktopException();
    SunDesktopException(int errorCode, const std::string& message) {}

    enum {
        ParseConfigurationError,
        ParseHEICError,
        OpenFileError,
        NetworkError,
        ParseJSONError
    };
};

#endif // EXCEPTION_H
