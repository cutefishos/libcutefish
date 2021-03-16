set(NETWORKMGR_SRCS
    appletproxymodel.cpp
    appletproxymodel.h

    networkitemslist.cpp
    networkitemslist.h

    networkmodel.cpp
    networkmodel.h

    networkmodelitem.cpp
    networkmodelitem.h

    networking.cpp
    networking.h

    network.cpp
    network.h

    wiressitemsettings.cpp
    wiressitemsettings.h

    technologyproxymodel.cpp
    technologyproxymodel.h

    uiutils.cpp
    uiutils.h

    connectionicon.cpp
    connectionicon.h

    qmlplugins.cpp
    qmlplugins.h
)

find_package(KF5NetworkManagerQt REQUIRED)
find_package(KF5ModemManagerQt REQUIRED)

add_library(cutefishnm_qmlplugins SHARED ${NETWORKMGR_SRCS})
generate_export_header(cutefishnm_qmlplugins EXPORT_FILE_NAME ${CMAKE_CURRENT_BINARY_DIR}/networkmanager_export.h BASE_NAME NetworkManager)

target_link_libraries (cutefishnm_qmlplugins
    Qt5::Core
    Qt5::Quick
    Qt5::Gui
    Qt5::DBus

    KF5::NetworkManagerQt
    KF5::ModemManagerQt
)

install(TARGETS cutefishnm_qmlplugins DESTINATION ${INSTALL_QMLDIR}/Cutefish/NetworkManagement)
install(FILES qmldir DESTINATION ${INSTALL_QMLDIR}/Cutefish/NetworkManagement)