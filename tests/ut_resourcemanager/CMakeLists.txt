set(TEST_SOURCES
    tst_resourcemanager.cpp
    ${CMAKE_SOURCE_DIR}/src/core/resourcemanager.cpp
    ${CMAKE_SOURCE_DIR}/src/core/resourceclient.cpp
    ${CMAKE_SOURCE_DIR}/src/util/logger.cpp
    ${CMAKE_SOURCE_DIR}/src/policy/securitypolicy.cpp
    ${CMAKE_SOURCE_DIR}/src/policy/prioritypolicy.cpp
)

add_executable(ut_resourcemanager ${TEST_SOURCES})

target_link_libraries(ut_resourcemanager
    PRIVATE
        Qt6::Test
        Qt6::DBus
)

target_include_directories(ut_resourcemanager PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

add_test(NAME ut_resourcemanager COMMAND ut_resourcemanager)
