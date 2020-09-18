#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <string>

class Exception
{
public:
    Exception();
    Exception(int errorCode, const std::string& message) {}

    enum {
        ParseConfigurationError,
        ParseHEICError,
        OpenFileError,
        NetworkError,
        ParseJSONError,
        WallpaperNotSetError,
    };

};

#endif // EXCEPTION_H
