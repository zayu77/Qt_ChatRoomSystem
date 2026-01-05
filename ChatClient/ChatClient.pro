QT       += core gui sql network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    addfrienddialog.cpp \
    chatclient.cpp \
    idatabase.cpp \
    loginview.cpp \
    main.cpp \
    mainwindow.cpp \
    masterview.cpp \
    messagedelegate.cpp \
    messagemodel.cpp \
    privatechat.cpp \
    registerview.cpp

HEADERS += \
    addfrienddialog.h \
    chatclient.h \
    idatabase.h \
    loginview.h \
    mainwindow.h \
    masterview.h \
    messagedelegate.h \
    messagemodel.h \
    privatechat.h \
    registerview.h

FORMS += \
    addfrienddialog.ui \
    loginview.ui \
    mainwindow.ui \
    masterview.ui \
    privatechat.ui \
    registerview.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    imgs.qrc
