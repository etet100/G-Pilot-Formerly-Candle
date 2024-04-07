#-------------------------------------------------
#
# Project created by QtCreator 2014-05-23T17:51:21
#
#-------------------------------------------------

QT       = core gui opengl serialport script uitools
equals(QT_MAJOR_VERSION, 5): QT += widgets network

win32: {
    QT += winextras
    DEFINES += WINDOWS
    QMAKE_CXXFLAGS_DEBUG += -g3 -pg
    QMAKE_LFLAGS_DEBUG += -pg -lgmon
}

unix:!macx {
    DEFINES += UNIX #GL_GLEXT_PROTOTYPES
    QMAKE_LFLAGS += "-Wl,-rpath,\'\$$ORIGIN/libs\'"
}

contains(QT_CONFIG, opengles.) {
    warning("GL ES detected. VAO will be disabled.")
    DEFINES += GLES
    INSTALLS += target
    target.path = /home/pi
}

TARGET = GPilot
TEMPLATE = app

RC_ICONS = images/gpilot.ico

DEFINES += sNan=\"65536\"

TRANSLATIONS += translations/candle_en.ts translations/candle_ru.ts translations/candle_es.ts translations/candle_fr.ts translations/candle_pt.ts

CONFIG += c++14
QMAKE_CXXFLAGS += -std=c++14

# don't create both debug and release folders
CONFIG -= debug_and_release

SUBDIRS += vendor/PropertyEditor

SOURCES += main.cpp\
    communicator.cpp \
    config/configuration.cpp \
    config/module/configurationconnection.cpp \
    config/module/configurationconsole.cpp \
    config/module/configurationsender.cpp \
    config/module/configurationvisualizer.cpp \
    config/persistence/ini/inipersister.cpp \
    config/persistence/ini/iniprovider.cpp \
    config/persistence/xml/xmlpersister.cpp \
    config/persistence/xml/xmlprovider.cpp \
    connection/connection.cpp \
    connection/connectionmanager.cpp \
    connection/rawtcpconnection.cpp \
    connection/serialconnection.cpp \
    connection/virtualucncconnection.cpp \
    form_partial/main/console.cpp \
    form_partial/main/control.cpp \
    form_partial/main/jog.cpp \
    form_partial/main/override.cpp \
    form_partial/main/state.cpp \
    frmgrblconfigurator.cpp \
    frmmain.cpp \
    linereceived.cpp \
    frmsettings.cpp \
    frmabout.cpp \
    drawers/gcodedrawer.cpp \
    drawers/heightmapborderdrawer.cpp \
    drawers/heightmapgriddrawer.cpp \
    drawers/heightmapinterpolationdrawer.cpp \
    drawers/origindrawer.cpp \
    drawers/shaderdrawable.cpp \
    drawers/tooldrawer.cpp \
    drawers/machineboundsdrawer.cpp \
    machine/machineconfiguration.cpp \
    parser/arcproperties.cpp \
    parser/gcodeparser.cpp \
    parser/gcodepreprocessorutils.cpp \
    parser/gcodeviewparse.cpp \
    parser/linesegment.cpp \
    parser/pointsegment.cpp \
    scripting/scripting.cpp \
    streamer.cpp \
    tables/gcodetablemodel.cpp \
    tables/heightmaptablemodel.cpp \
    widgets/combobox.cpp \
    widgets/comboboxkey.cpp \
    widgets/glwidget.cpp \
    drawers/selectiondrawer.cpp \
    scripting/scriptvars.cpp \
    scripting/scriptfunctions.cpp \
    widgets/dropwidget.cpp

HEADERS  += frmmain.h \
    communicator.h \
    config/configuration.h \
    config/module/configurationconnection.h \
    config/module/configurationconsole.h \
    config/module/configurationsender.h \
    config/module/configurationvisualizer.h \
    config/module/module.h \
    config/persistence/persister.h \
    config/persistence/provider.h \
    config/persistence/ini/inipersister.h \
    config/persistence/ini/iniprovider.h \
    config/persistence/xml/xmlpersister.h \
    config/persistence/xml/xmlprovider.h \
    connection/connection.h \
    connection/connectionmanager.h \
    connection/rawtcpconnection.h \
    connection/serialconnection.h \
    connection/virtualucncconnection.h \
    form_partial/main/console.h \
    form_partial/main/control.h \
    form_partial/main/jog.h \
    form_partial/main/override.h \
    form_partial/main/state.h \
    frmgrblconfigurator.h \
    frmsettings.h \
    frmabout.h \
    drawers/gcodedrawer.h \
    drawers/heightmapborderdrawer.h \
    drawers/heightmapgriddrawer.h \
    drawers/heightmapinterpolationdrawer.h \
    drawers/origindrawer.h \
    drawers/shaderdrawable.h \
    drawers/tooldrawer.h \
    drawers/machineboundsdrawer.h \
    globals.h \
    machine/machineconfiguration.h \
    parser/arcproperties.h \
    parser/gcodeparser.h \
    parser/gcodepreprocessorutils.h \
    parser/gcodeviewparse.h \
    parser/linesegment.h \
    parser/pointsegment.h \
    scripting/scripting.h \
    streamer.h \
    tables/gcodetablemodel.h \
    tables/heightmaptablemodel.h \
    utils/interpolation.h \
    utils/util.h \
    widgets/combobox.h \
    widgets/comboboxkey.h \
    widgets/glwidget.h \
    drawers/selectiondrawer.h \
    scripting/scriptvars.h \
    scripting/scriptfunctions.h \
    widgets/dropwidget.h

FORMS    += frmmain.ui \
    form_partial/main/console.ui \
    form_partial/main/control.ui \
    form_partial/main/jog.ui \
    form_partial/main/override.ui \
    form_partial/main/state.ui \
    frmdebug.ui \
    frmgrblconfigurator.ui \
    frmsettings.ui \
    frmabout.ui

DEFINES += _USE_MATH_DEFINES

RESOURCES += \
    shaders.qrc \
    images.qrc

INCLUDEPATH += ../designerplugins/customwidgetsplugin
INCLUDEPATH += ../vendor/PropertyEditor

LIBS += -L../designerplugins/customwidgetsplugin -lcustomwidgets
LIBS += -L../vendor/PropertyEditor -lPropertyEditor

# qtPrepareTool(LRELEASE, lrelease)
# for(tsfile, TRANSLATIONS) {
#     qmfile = $$tsfile
#     qmfile ~= s,.ts$,.qm,
#     qmdir = $$dirname(qmfile)
#     !exists($$qmdir) {
#         mkpath($$qmdir)|error("Aborting.")
#     }
#     command = $$LRELEASE -removeidentical $$tsfile -qm $$qmfile
#     system($$command)|error("Failed to run: $$command")
# }

LIBS += -L../vendor/uCNC -luCNC

INCLUDEPATH += $$PWD/../vendor/uCNC/build
INCLUDEPATH += $$PWD/../vendor/uCNC/makefiles/virtual
DEPENDPATH += ../vendor/uCNC

# win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../vendor/uCNC/release/libuCNC.a
# else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../vendor/uCNC/release/uCNC.lib

# win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../vendor/uCNC/release/ -luCNC
# else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../vendor/uCNC/debug/ -luCNC

# INCLUDEPATH += $$PWD/../vendor/uCNC/build
# DEPENDPATH += $$PWD/../vendor/uCNC/build

# win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../vendor/uCNC/release/libuCNC.a
# else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../vendor/uCNC/debug/libuCNC.a
# else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../vendor/uCNC/release/uCNC.lib
# else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../vendor/uCNC/debug/uCNC.lib

win32: LIBS += -luCNC
