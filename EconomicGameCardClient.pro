#-------------------------------------------------
#
# Project created by QtCreator 2023-08-18T14:49:14
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = EconomicGameCardClient
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

unix:!mac{
  QMAKE_LFLAGS += -Wl,--rpath=\\\$\$ORIGIN
  QMAKE_LFLAGS += -Wl,--rpath=\\\$\$ORIGIN/lib
  QMAKE_LFLAGS += -Wl,--rpath=\\\$\$ORIGIN/plugins
}

CONFIG += c++14 -lnfc

LIBS += -L/usr/local/lib -lnfc
INCLUDEPATH += \
    headers \
    /usr/local/include

SOURCES += \
    main.cpp \
    cpp/bankerwindow.cpp \
    cpp/banknfcwindow.cpp \
    cpp/companywindow.cpp \
    cpp/customwindow.cpp \
    cpp/authwindow.cpp \
    cpp/requestsender.cpp \
    cpp/teacherwindow.cpp \
    cpp/servicepaywindow.cpp \
    cpp/nfcmanager.cpp \
    cpp/atmwindow.cpp \

HEADERS += \
    headers/banknfcwindow.h \
    headers/bankerwindow.h \
    headers/companywindow.h \
    headers/authwindow.h \
    headers/customwindow.h \
    headers/requestsender.h \
    headers/teacherwindow.h \
    headers/servicepaywindow.h \
    headers/nfcmanager.h \
    headers/atmwindow.h \

FORMS += \
    forms/banknfcwindow.ui \
    forms/bankerwindow.ui \
    forms/companywindow.ui \
    forms/authwindow.ui \
    forms/teacherwindow.ui \
    forms/servicepaywindow.ui \
    forms/atmwindow.ui \

RESOURCES += \
    res/resources.qrc


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES +=
