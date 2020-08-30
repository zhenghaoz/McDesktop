cmake_minimum_required(VERSION 3.5)

project(ddesktop LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 COMPONENTS Widgets LinguistTools REQUIRED)
find_package(Qt5Xml REQUIRED)
find_package(Boost REQUIRED)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/third_party/libheif/cmake/modules")

# libheif

add_subdirectory(third_party/libheif)

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

add_executable(ddesktop
  src/main.cpp
  src/mainwindow.cpp
  src/mainwindow.h
  src/wallpaper.cpp
  src/wallpaper.h
  src/engine.cpp
  src/engine.h
)

target_include_directories(ddesktop PRIVATE src/PlistCpp/src)
target_include_directories(ddesktop PRIVATE src/SolTrack)
target_include_directories(ddesktop PRIVATE third_party/cpp-httplib)

target_link_libraries(ddesktop PRIVATE Qt5::Widgets Qt5::Xml heif pthread SolTrack PlistCpp)
