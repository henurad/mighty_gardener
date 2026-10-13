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
    target_sources(MightyGardenerLib PUBLIC hardware/pigpio-compat.cpp)
endif()

target_sources(MightyGardenerLib
    PUBLIC
        hardware/relay.cpp
        communication/serial-port.cpp
        communication/sms-utils.cpp
        communication/sms-manager.cpp
        control/relay-scheduler.cpp
        hardware/dht11-sensor.cpp
        hardware/am2302-sensor.cpp
        hardware/yf-s201.cpp
        hardware/soil-moisture-sensor.cpp
        hardware/light-sensor.cpp
        config/app-config.cpp

    PUBLIC
        FILE_SET HEADERS
        FILES
            hardware/relay.h
            hardware/dht11-sensor.h
            hardware/am2302-sensor.h
            hardware/yf-s201.h
            hardware/soil-moisture-sensor.h
            hardware/light-sensor.h
            hardware/pigpio.h
            communication/serial-port.h
            communication/sms-utils.h
            communication/sms-manager.h
            communication/time-consts.h
            control/relay-scheduler.h
            config/mini-yaml.h
            config/app-config.h
)

add_executable(MightyGardener main.cpp)
target_link_libraries(MightyGardener PRIVATE MightyGardenerLib)
