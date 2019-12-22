TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt


INCLUDEPATH += /usr/local/include \
               /usr/local/include/opencv4

LIBS += /usr/local/lib/libopencv_highgui.so\
        /usr/local/lib/libopencv_core.so\
        /usr/local/lib/libopencv_imgproc.so\
        /usr/local/lib/libopencv_imgcodecs.so\
        /usr/local/lib/libopencv_video.so\
        /usr/local/lib/libopencv_videoio.so\
        /usr/local/lib/libopencv_photo.so

SOURCES += \
    main.cpp

HEADERS += \
    opencv_extended.h \
    armordetector.h
