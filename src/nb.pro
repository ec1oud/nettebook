QT += core gui widgets network
TARGET = nb
TEMPLATE = app
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11
# qmake -config no_kio
no_kio: DEFINES += NETTEBOOK_NO_KIO

SOURCES += \
    cidfinder.cpp \
    document.cpp \
    ipfsagent.cpp \
    linkdialog.cpp \
    main.cpp \
    mainwindow.cpp \
    markdownbrowser.cpp \
    jsonview.cpp \
    ../deps/QJsonModel/qjsonmodel.cpp \
    settings.cpp \
    settingsdialog.cpp \
    tablesizedialog.cpp \
    tableviewdialog.cpp \
    thumbnailitem.cpp \
    thumbnailscene.cpp \
    thumbnailview.cpp \

HEADERS += \
    cidfinder.h \
    document.h \
    ipfsagent.h \
    linkdialog.h \
    mainwindow.h \
    markdownbrowser.h \
    jsonview.h \
    ../deps/QJsonModel/qjsonmodel.h \
    settings.h \
    settingsdialog.h \
    tablesizedialog.h \
    tableviewdialog.h \
    thumbnailitem.h \
    thumbnailscene.h \
    thumbnailview.h \

FORMS += \
    linkdialog.ui \
    mainwindow.ui \
    settingsdialog.ui \
    tablesizedialog.ui \
    tableviewdialog.ui

RESOURCES += \
    resources/resources.qrc

INCLUDEPATH += /usr/include/KF5/KIOCore ../deps/QJsonModel
!no_kio: INCLUDEPATH += /usr/include/KF5/KCoreAddons
!no_kio: LIBS += -L/usr/lib/kf5 -lKF5KIOCore -lKF5CoreAddons

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
