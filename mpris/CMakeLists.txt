set(NETWORKMGR_SRCS
    mpris.cpp
    mpris.h
    mpriscontroller_p.h
    mpriscontroller.cpp
    mpriscontroller.h
    mprismanager.cpp
    mprismanager.h
    mprisplayer.cpp
    mprisplayer.h
    mprisplayeradaptor.cpp
    mprisplayerinterface.cpp
    mprisqt.h
    mprisrootadaptor.cpp
    mprisrootinterface.cpp
    qmlplugins.cpp

    qtdbusextended/dbusextended.h
    qtdbusextended/dbusextendedabstractinterface.cpp
    qtdbusextended/dbusextendedabstractinterface.h
    qtdbusextended/dbusextendedpendingcallwatcher_p.h
    qtdbusextended/dbusextendedpendingcallwatcher.cpp
)

add_library(cutefishmpris_qmlplugins SHARED ${NETWORKMGR_SRCS})

target_link_libraries (cutefishmpris_qmlplugins
    Qt5::Core
    Qt5::Quick
    Qt5::Gui
    Qt5::DBus
)

install(TARGETS cutefishmpris_qmlplugins DESTINATION ${INSTALL_QMLDIR}/Cutefish/Mpris)
install(FILES qmldir DESTINATION ${INSTALL_QMLDIR}/Cutefish/Mpris)