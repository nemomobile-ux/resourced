set(TEST_SOURCES
    tst_manageradaptor.cpp
    ${CMAKE_SOURCE_DIR}/src/core/resourcemanager.cpp
    ${CMAKE_SOURCE_DIR}/src/core/resourceclient.cpp
    ${CMAKE_SOURCE_DIR}/src/util/logger.cpp
    ${CMAKE_SOURCE_DIR}/src/policy/securitypolicy.cpp
    ${CMAKE_SOURCE_DIR}/src/policy/prioritypolicy.cpp
    ${CMAKE_SOURCE_DIR}/src/dbus/manageradaptor.cpp
    ${CMAKE_SOURCE_DIR}/src/dbus/clientadaptor.cpp
)

add_executable(ut_manageradaptor ${TEST_SOURCES})

target_link_libraries(ut_manageradaptor
    PRIVATE
        Qt6::Test
        Qt6::DBus
)

target_include_directories(ut_manageradaptor PRIVATE
    ${CMAKE_SOURCE_DIR}/src
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
    target_include_directories(ut_manageradaptor PRIVATE "${_qdbus_base}" "${_qdbus_dir}" "${_qdbus_private_dir}")
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
    target_include_directories(ut_manageradaptor PRIVATE "${_qcore_base}" "${_qcore_dir}" "${_qcore_private_dir}")
endif()

add_test(NAME ut_manageradaptor COMMAND ut_manageradaptor)
