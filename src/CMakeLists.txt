find_package(Qt6 REQUIRED COMPONENTS Core DBus)

set(SRCS
    main.cpp
    core/resourcemanager.cpp
    core/resourceclient.cpp
    dbus/clientadaptor.cpp
    dbus/manageradaptor.cpp
    policy/securitypolicy.cpp
    policy/prioritypolicy.cpp
    util/logger.cpp
)

set(HEADERS
    core/resourcemanager.h
    core/resourceclient.h
    dbus/manageradaptor.h
    dbus/clientadaptor.h
    policy/securitypolicy.h
    policy/prioritypolicy.h
    util/logger.h)

add_executable(resourced
    ${SRCS}
    ${HEADERS}
    ${GENERATED_SOURCES}
)

target_include_directories(resourced PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)
find_path(QDBUSMESSAGE_P_H qdbusmessage_p.h
    HINTS /usr/include/x86_64-linux-gnu/qt6
    PATH_SUFFIXES
        QtDBus/6.10.2/QtDBus/private
        QtDBus/6.9.0/QtDBus/private
        QtDBus/6.8.3/QtDBus/private
        QtDBus/6.8.0/QtDBus/private
        QtDBus/6.7.2/QtDBus/private
        QtDBus/6.7.0/QtDBus/private
        QtDBus/6.6.3/QtDBus/private
        QtDBus/6.6.0/QtDBus/private
        QtDBus/6.5.3/QtDBus/private
        QtDBus/6.5.0/QtDBus/private
        QtDBus/6.4.3/QtDBus/private
        QtDBus/6.4.0/QtDBus/private
        QtDBus/6.3.2/QtDBus/private
        QtDBus/6.3.0/QtDBus/private
        QtDBus/6.2.4/QtDBus/private
        QtDBus/6.2.0/QtDBus/private
)
if(QDBUSMESSAGE_P_H)
    set(_qdbus_private_dir "${QDBUSMESSAGE_P_H}")
    get_filename_component(_qdbus_dir "${_qdbus_private_dir}" DIRECTORY)
    get_filename_component(_qdbus_base "${_qdbus_dir}" DIRECTORY)
    target_include_directories(resourced PRIVATE "${_qdbus_base}" "${_qdbus_dir}" "${_qdbus_private_dir}")
endif()

find_path(QGLOBAL_P_H qglobal_p.h
    HINTS /usr/include/x86_64-linux-gnu/qt6
    PATH_SUFFIXES
        QtCore/6.10.2/QtCore/private
        QtCore/6.9.0/QtCore/private
        QtCore/6.8.3/QtCore/private
        QtCore/6.8.0/QtCore/private
        QtCore/6.7.2/QtCore/private
        QtCore/6.7.0/QtCore/private
        QtCore/6.6.3/QtCore/private
        QtCore/6.6.0/QtCore/private
        QtCore/6.5.3/QtCore/private
        QtCore/6.5.0/QtCore/private
        QtCore/6.4.3/QtCore/private
        QtCore/6.4.0/QtCore/private
        QtCore/6.3.2/QtCore/private
        QtCore/6.3.0/QtCore/private
        QtCore/6.2.4/QtCore/private
        QtCore/6.2.0/QtCore/private
)
if(QGLOBAL_P_H)
    set(_qcore_private_dir "${QGLOBAL_P_H}")
    get_filename_component(_qcore_dir "${_qcore_private_dir}" DIRECTORY)
    get_filename_component(_qcore_base "${_qcore_dir}" DIRECTORY)
    target_include_directories(resourced PRIVATE "${_qcore_base}" "${_qcore_dir}" "${_qcore_private_dir}")
endif()

target_link_libraries(resourced
    Qt6::Core
    Qt6::DBus
)

install(TARGETS resourced
    LIBRARY DESTINATION ${CMAKE_INSTALL_BINDIR}
)
