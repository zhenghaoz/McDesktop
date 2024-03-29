cmake_minimum_required(VERSION 3.5)

project(sundesktop LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 COMPONENTS Widgets LinguistTools REQUIRED)
find_package(Qt5Xml REQUIRED)
find_package(Boost REQUIRED)
find_package(Threads REQUIRED)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/third_party/libheif/cmake/modules")

add_subdirectory(third_party/libheif)
add_subdirectory(third_party/spdlog)
add_subdirectory(third_party/googletest)

# SolTrack

set_source_files_properties(src/SolTrack/SolTrack.c PROPERTIES LANGUAGE CXX)
set_source_files_properties(src/SolTrack/SolTrack_RiseSet.c PROPERTIES LANGUAGE CXX)

add_library(SolTrack STATIC
  src/SolTrack/SolTrack.h
  src/SolTrack/SolTrack.c
  src/SolTrack/SolTrack_RiseSet.c)

# PlistCpp

add_library(PlistCpp STATIC
  src/PlistCpp/src/pugixml.hpp
  src/PlistCpp/src/Plist.hpp
  src/PlistCpp/src/PlistDate.hpp
  src/PlistCpp/src/pugixml.cpp
  src/PlistCpp/src/Plist.cpp
  src/PlistCpp/src/PlistDate.cpp
)

add_executable(sundesktop
  src/main.cpp
  src/mainwindow.cpp
  src/mainwindow.h
  src/heic.cpp
  src/heic.h
  src/cache.cpp
  src/cache.h
  src/exception.cpp
  src/exception.h
  src/parser.cpp
  src/parser.h
  src/daemon.cpp
  src/daemon.h
  src/desktop.cpp
  src/desktop.h
)

target_include_directories(sundesktop PRIVATE src/PlistCpp/src)
target_include_directories(sundesktop PRIVATE src/SolTrack)
target_include_directories(sundesktop PRIVATE third_party/cpp-httplib)
target_include_directories(sundesktop PRIVATE third_party/libheif)
target_include_directories(sundesktop PRIVATE third_party/spdlog/spdlog)
target_include_directories(sundesktop PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/third_party/libheif)
target_include_directories(sundesktop PRIVATE ${Boost_INCLUDE_DIRS})

target_include_directories(PlistCpp PRIVATE ${Boost_INCLUDE_DIRS})

target_link_libraries(sundesktop PRIVATE Qt5::Widgets Qt5::Xml heif SolTrack PlistCpp spdlog::spdlog Threads::Threads)
