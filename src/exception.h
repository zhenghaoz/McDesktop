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
        OpenFileError
    };
};

#endif // EXCEPTION_H
