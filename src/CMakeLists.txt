add_library(MightyGardenerLib)

target_include_directories(MightyGardenerLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(MightyGardenerLib PUBLIC cxx_std_11)

option(MIGHTY_GARDENER_REQUIRE_HARDWARE
       "Require Raspberry Pi GPIO libraries for a hardware deployment" OFF)

find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(GPIOD QUIET IMPORTED_TARGET libgpiod)
endif()

if(GPIOD_FOUND)
    target_link_libraries(MightyGardenerLib PUBLIC PkgConfig::GPIOD)
    target_compile_definitions(MightyGardenerLib PUBLIC HAVE_GPIOD)
endif()

find_library(PIGPIO_LIB NAMES pigpio)
find_path(PIGPIO_INCLUDE_DIR NAMES pigpio.h)

if(MIGHTY_GARDENER_REQUIRE_HARDWARE AND NOT GPIOD_FOUND)
    message(FATAL_ERROR
        "libgpiod was not found. Install libgpiod-dev for Raspberry Pi GPIO support.")
endif()

if(MIGHTY_GARDENER_REQUIRE_HARDWARE AND (NOT PIGPIO_LIB OR NOT PIGPIO_INCLUDE_DIR))
    message(FATAL_ERROR
        "pigpio headers or library were not found. Install libpigpio-dev for Raspberry Pi sensor support.")
endif()

target_sources(MightyGardenerLib
    PRIVATE
        hardware/relay.cpp
        communication/serial-port.cpp
        communication/sms-utils.cpp
        communication/sms-manager.cpp
        control/relay-scheduler.cpp
        control/schmitt-trigger-relay.cpp
        hardware/soil-moisture-sensor.cpp
        hardware/light-sensor.cpp
        config/app-config.cpp

    PUBLIC
        hardware/relay.h
        hardware/dht11-sensor.h
        hardware/am2302-sensor.h
        hardware/yf-s201.h
        hardware/soil-moisture-sensor.h
        hardware/light-sensor.h
        communication/serial-port.h
        communication/sms-utils.h
        communication/sms-manager.h
        communication/time-consts.h
        control/relay-scheduler.h
        control/schmitt-trigger-relay.h
        config/mini-yaml.h
        config/app-config.h
)

if(PIGPIO_LIB AND PIGPIO_INCLUDE_DIR)
    target_sources(MightyGardenerLib PRIVATE
        hardware/dht11-sensor.cpp
        hardware/am2302-sensor.cpp
        hardware/yf-s201.cpp
    )
    target_include_directories(MightyGardenerLib PRIVATE ${PIGPIO_INCLUDE_DIR})
    target_link_libraries(MightyGardenerLib PUBLIC ${PIGPIO_LIB})
endif()

if(GPIOD_FOUND AND PIGPIO_LIB AND PIGPIO_INCLUDE_DIR)
    add_executable(MightyGardener main.cpp)
    target_link_libraries(MightyGardener PRIVATE MightyGardenerLib)
else()
    message(STATUS
        "GPIO libraries not found; skipping MightyGardener hardware executable")
endif()
