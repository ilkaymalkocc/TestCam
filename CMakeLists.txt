cmake_minimum_required(VERSION 3.16)
project(TestCam)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_PREFIX_PATH "C:/Qt/5.15.2/mingw81_64")


# Qt
find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets)

# OpenCV
set(OpenCV_DIR "C:/opencv-build/install/x64/mingw/lib")
find_package(OpenCV REQUIRED)

include_directories("C:/opencv-build/install/include")
include_directories("C:/opencv-build/install/include/opencv2")

# Sources
set(SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui

)
configure_file(haarcascade_frontalface_default.xml haarcascade_frontalface_default.xml COPYONLY)

# Executable
add_executable(TestCam
    ${SOURCES}
)

target_link_libraries(TestCam
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    ${OpenCV_LIBS}
)
