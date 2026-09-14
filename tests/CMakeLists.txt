add_executable(TestSmsUtils)
target_link_libraries(TestSmsUtils PRIVATE MightyGardenerLib)
target_sources(TestSmsUtils PRIVATE test-sms-utils.cpp)

function(SmsUtilsUnitTest unit)
    add_test(
        NAME ${unit}
        COMMAND TestSmsUtils ${unit}
    )
endfunction()

# todo: add tests for sms-utils here
SmsUtilsUnitTest(ValidatePhoneNumber)
