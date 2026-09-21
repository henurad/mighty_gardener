add_library(MightyGardenerLib)

target_include_directories(MightyGardenerLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(GPIOD QUIET IMPORTED_TARGET libgpiod)
endif()

if(GPIOD_FOUND)
    target_link_libraries(MightyGardenerLib PUBLIC PkgConfig::GPIOD)
    target_compile_definitions(MightyGardenerLib PUBLIC HAVE_GPIOD)
endif()

find_library(PIGPIO_LIB NAMES pigpio)
if(PIGPIO_LIB)
    target_link_libraries(MightyGardenerLib PUBLIC ${PIGPIO_LIB})
else()
    target_sources(MightyGardenerLib PUBLIC pigpio-compat.cpp)
endif()

target_sources(MightyGardenerLib
    PUBLIC
        relay.cpp
        serial-port.cpp
        sms-utils.cpp
        sms-manager.cpp
        sun-relay.cpp
        dht11-sensor.cpp

    PUBLIC
        FILE_SET HEADERS
        FILES
            relay.h
            serial-port.h
            sms-utils.h
            sms-manager.h
            sun-relay.h
            dht11-sensor.h
            time-consts.h
            pigpio.h
)

add_executable(MightyGardener main.cpp)
target_link_libraries(MightyGardener PRIVATE MightyGardenerLib)
