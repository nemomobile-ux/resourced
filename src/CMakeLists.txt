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

target_link_libraries(resourced
    Qt6::Core
    Qt6::DBus
)

install(TARGETS resourced
    LIBRARY DESTINATION ${CMAKE_INSTALL_BINDIR}
)
