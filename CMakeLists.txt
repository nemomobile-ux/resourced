cmake_minimum_required(VERSION 3.16)
project(resourced)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(QT_MIN_VERSION "6.2.0")
set(CMAKE_INSTALL_PREFIX /usr)

include(GNUInstallDirs)

find_package(Qt6 ${QT_MIN_VERSION} REQUIRED COMPONENTS
    Core
    DBus)

add_subdirectory(src)
add_subdirectory(tests)
