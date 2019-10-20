# Qt configuration file
# Run `qmake` once, then `make`.

QT += widgets
CONFIG += c++11

HEADERS += \
        objects.hpp

SOURCES += \
        collider.cpp \
        objects.cpp

RESOURCES += \
        collider.qrc

DISTFILES += \
    README.md


