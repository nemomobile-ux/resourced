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

add_test(NAME ut_manageradaptor COMMAND ut_manageradaptor)
