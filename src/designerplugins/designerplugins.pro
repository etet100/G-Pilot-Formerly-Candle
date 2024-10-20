TEMPLATE = subdirs
 
SUBDIRS = cameraplugin customwidgetsplugin joystickplugin

install_cameraplugin.path = ../../bin/plugins/camera/plugins
install_cameraplugin.files = cameraplugin/cameraplugin.dll

install_customwidgetsplugin.path = ../../bin
install_customwidgetsplugin.files = customwidgetsplugin/customwidgets.dll

install_joystickplugin.path = ../../bin/plugins/joystick/plugins
install_joystickplugin.files = joystickplugin/joystickplugin.dll

INSTALLS += install_cameraplugin install_customwidgetsplugin install_joystickplugin

CONFIG += c++14
QMAKE_CXXFLAGS += -std=c++14
