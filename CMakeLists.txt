cmake_minimum_required(VERSION 3.5)

project(ddesktop LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 COMPONENTS Widgets LinguistTools REQUIRED)

add_executable(ddesktop
  src/main.cpp
  src/mainwindow.cpp
  src/mainwindow.h
)

target_link_libraries(ddesktop PRIVATE Qt5::Widgets)
