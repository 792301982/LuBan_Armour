TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
        main.cpp

INCLUDEPATH += /usr/include \
               /usr/include/opencv \
               /usr/include/opencv2

LIBS += /usr/lib/libopencv_highgui.so \
        /usr/lib/libopencv_core.so    \
        /usr/lib/libopencv_imgproc.so \
        /usr/lib/libopencv_imgcodecs.so \
        /usr/lib/libopencv_video.so \
        /usr/lib/libopencv_videoio.so

HEADERS += \
    armor.h
