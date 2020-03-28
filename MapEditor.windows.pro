#-------------------------------------------------
#
# Project created by QtCreator 2019-02-25T08:26:33
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MapEditor
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

LIBS += \
     -L"C:/Program Files (x86)/FMOD SoundSystem/FMOD Studio API Windows/api/core/lib/x64" \
     -L"C:/Program Files (x86)/FMOD SoundSystem/FMOD Studio API Windows/api/studio/lib/x64" \
    -lglu32 -lfmodstudio_vc -lfmod_vc

INCLUDEPATH += \
        ../../Libraries/glm \
        ../../Libraries/glew/include \
       "C:/Program Files (x86)/FMOD SoundSystem/FMOD Studio API Windows/api/core/inc" \
       "C:/Program Files (x86)/FMOD SoundSystem/FMOD Studio API Windows/api/studio/inc"

CONFIG += c++14
QMAKE_CXXFLAGS += -d2FH4-
QMAKE_LFLAGS +=  -d2:-FH4-

DEFINES += GLM_EXT_INCLUDED

SOURCES += \
        main.cpp \
        mainwindow.cpp \
	src/Shaders/arrowshader.cpp \
    src/document.cpp \
    src/commandlist.cpp \
	src/fluidlinks.cpp \
    src/glviewwidget.cpp \
    src/fmodmanager.cpp \
    src/fmoderror.cpp \
    src/backgroundimage.cpp \
    src/glprogram.cpp \
    src/Shaders/transparencyshader.cpp \
    src/Shaders/defaulttextures.cpp \
    src/Shaders/defaultvaos.cpp \
    src/Shaders/blitshader.cpp \
    src/metaroom.cpp \
    src/lf_math.cpp \
    src/metaroomselection.cpp \
    src/quadtree.cpp \
    src/metaroomdoors.cpp \
    src/controllerfsm.cpp \
    src/Shaders/uniformcolorshader.cpp \
    src/indexbuffers.cpp \
    src/Shaders/selectedroomshader.cpp \
    src/Shaders/roomoutlineshader.cpp \
    src/Shaders/doorshader.cpp \
    src/clipboard.cpp \
	src/reseatalgorithm.cpp \
	src/walltype_db.cpp

HEADERS += \
        mainwindow.h \
	nlohmann/json_detail.hpp \
	src/Shaders/arrowshader.h \
    src/document.h \
    src/commandlist.h \
	src/enums.hpp \
	src/fluidlinks.h \
    src/glviewwidget.h \
    src/fmodmanager.h \
    src/fmoderror.h \
    src/backgroundlayer.h \
    src/backgroundimage.h \
    src/byte_swap.h \
    src/glprogram.h \
    src/Shaders/transparencyshader.h \
    src/Shaders/defaulttextures.h \
    src/Shaders/defaultvaos.h \
    src/Shaders/blitshader.h \
    src/metaroom.h \
    src/lf_math.h \
    src/metaroomselection.h \
    src/quadtree.h \
    src/metaroomdoors.h \
    src/controllerfsm.h \
    src/Shaders/uniformcolorshader.h \
    src/indexbuffers.h \
    src/Shaders/selectedroomshader.h \
    src/Shaders/roomoutlineshader.h \
    src/Shaders/doorshader.h \
    src/clipboard.h \
	src/reseatalgorithm.h \
	src/walltype_db.h

FORMS += \
        mainwindow.ui

# Default rules for deployment.
#qnx: target.path = /tmp/$${TARGET}/bin
#else: unix:!android: target.path = /opt/$${TARGET}/bin
#!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    C:/Program Files (x86)/FMOD SoundSystem/FMOD Studio API Windows/api/core/inc/fmod.cs \
    C:/Program Files (x86)/FMOD SoundSystem/FMOD Studio API Windows/api/core/inc/fmod_dsp.cs \
    C:/Program Files (x86)/FMOD SoundSystem/FMOD Studio API Windows/api/core/inc/fmod_errors.cs \
    C:/Program Files (x86)/FMOD SoundSystem/FMOD Studio API Windows/api/core/lib/x64/fmod.dll \
    C:/Program Files (x86)/FMOD SoundSystem/FMOD Studio API Windows/api/core/lib/x64/fmodL.dll \
    C:/Program Files (x86)/FMOD SoundSystem/FMOD Studio API Windows/api/core/lib/x64/fmodL_vc.lib \
    C:/Program Files (x86)/FMOD SoundSystem/FMOD Studio API Windows/api/core/lib/x64/fmod_vc.lib \
    C:/Program Files (x86)/FMOD SoundSystem/FMOD Studio API Windows/api/fsbank/lib/x64/fsbank.dll \
    C:/Program Files (x86)/FMOD SoundSystem/FMOD Studio API Windows/api/fsbank/lib/x64/fsbank_vc.lib \
    C:/Program Files (x86)/FMOD SoundSystem/FMOD Studio API Windows/api/fsbank/lib/x64/libfsbvorbis64.dll \
    C:/Program Files (x86)/FMOD SoundSystem/FMOD Studio API Windows/api/studio/inc/fmod_studio.cs \
    C:/Program Files (x86)/FMOD SoundSystem/FMOD Studio API Windows/api/studio/lib/x64/fmodstudio.dll \
    C:/Program Files (x86)/FMOD SoundSystem/FMOD Studio API Windows/api/studio/lib/x64/fmodstudioL.dll \
    C:/Program Files (x86)/FMOD SoundSystem/FMOD Studio API Windows/api/studio/lib/x64/fmodstudioL_vc.lib \
    C:/Program Files (x86)/FMOD SoundSystem/FMOD Studio API Windows/api/studio/lib/x64/fmodstudio_vc.lib
