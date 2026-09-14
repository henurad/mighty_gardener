add_library(MightyGardenerLib)
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
