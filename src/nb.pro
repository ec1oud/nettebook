QT += core gui widgets network
TARGET = nb
TEMPLATE = app
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
    cidfinder.cpp \
    document.cpp \
    ipfsagent.cpp \
    main.cpp \
    mainwindow.cpp \
    markdownbrowser.cpp \
    jsonview.cpp \
    ../deps/QJsonModel/qjsonmodel.cpp \
    tablesizedialog.cpp \
    thumbnailitem.cpp \
    thumbnailscene.cpp \
    thumbnailview.cpp \

HEADERS += \
    cidfinder.h \
    document.h \
    ipfsagent.h \
    mainwindow.h \
    markdownbrowser.h \
    jsonview.h \
    ../deps/QJsonModel/qjsonmodel.h \
    tablesizedialog.h \
    thumbnailitem.h \
    thumbnailscene.h \
    thumbnailview.h \

FORMS += \
    mainwindow.ui \
    tablesizedialog.ui

RESOURCES += \
    resources/resources.qrc

INCLUDEPATH += /usr/include/KF5/KIOCore ../deps/QJsonModel
LIBS += -L/usr/lib/kf5 -lKF5KIOCore -lKF5CoreAddons

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
