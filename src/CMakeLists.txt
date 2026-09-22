add_library(MightyGardenerLib)

find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(GPIOD QUIET IMPORTED_TARGET libgpiod)
endif()

if(GPIOD_FOUND)
    target_link_libraries(MightyGardenerLib PUBLIC PkgConfig::GPIOD)
    target_compile_definitions(MightyGardenerLib PUBLIC HAVE_GPIOD)
endif()

target_include_directories(MightyGardenerLib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_sources(MightyGardenerLib
    PUBLIC
        relay.cpp
        serial-port.cpp
        sms-utils.cpp

    PUBLIC
        FILE_SET HEADERS
        FILES
            relay.h
            serial-port.h
            sms-utils.h
            dht11-sensor.h
            time-consts.h
)

add_executable(MightyGardener main.cpp)
target_link_libraries(MightyGardener PRIVATE MightyGardenerLib)
