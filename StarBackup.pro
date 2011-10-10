#-------------------------------------------------
#
# Project created by QtCreator 2010-10-06T18:43:59
#
#-------------------------------------------------

QT       += core gui xml

TARGET = StarBackup
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp \
    checkablefoldertree.cpp \
    onlycheckedfoldertree.cpp \
    profilecreator.cpp \
    backupsettings.cpp \
    backuplistmodel.cpp \
    destinationwidget.cpp \
    folderlist.cpp \
    backupengine.cpp \
    destfolderinfo.cpp \
    filesystemtools.cpp \
    backuplistwidget.cpp \
    instancemanager.cpp \
    folder.cpp \
    languagechooser.cpp

HEADERS  += mainwindow.h \
    checkablefoldertree.h \
    trace.h \
    onlycheckedfoldertree.h \
    profilecreator.h \
    backupsettings.h \
    backuplistmodel.h \
    destinationwidget.h \
    folderlist.h \
    backupengine.h \
    destfolderinfo.h \
    filesystemtools.h \
    backuplistwidget.h \
    instancemanager.h \
    folder.h \
    languagechooser.h

FORMS    += mainwindow.ui \
    profilecreator.ui \
    languagechooser.ui

RESOURCES += \
    resources.qrc

RC_FILE = icon.rc


TRANSLATIONS = starbackup_fr.ts \
               starbackup_it.ts \
               starbackup_zh.ts \
               starbackup_es.ts

OTHER_FILES += \
    COPYING \
    README

#CONFIG += static
static { # everything below takes effect with CONFIG += static
    CONFIG += static
    DEFINES += STATIC
    message("~~~ static build ~~~") # this is for information, that the static build is done
#    win32: TARGET = $$join(TARGET,,,s) #this adds an s in the end, so you can seperate static build from non static build
}






