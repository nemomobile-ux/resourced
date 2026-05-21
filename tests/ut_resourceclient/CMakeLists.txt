set(TEST_SOURCES
    tst_resourceclient.cpp
    ${CMAKE_SOURCE_DIR}/src/core/resourceclient.cpp
)

add_executable(ut_resourceclient ${TEST_SOURCES})

target_link_libraries(ut_resourceclient
    PRIVATE
        Qt6::Test
        Qt6::DBus
)

target_include_directories(ut_resourceclient PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

add_test(NAME ut_resourceclient COMMAND ut_resourceclient)
