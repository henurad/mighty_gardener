find_package(GTest QUIET)

if(NOT GTest_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
endif()

add_executable(TestSmsUtils test-sms-utils.cpp)
target_link_libraries(TestSmsUtils PRIVATE MightyGardenerLib GTest::gtest_main)

add_executable(TestSmsManager test-sms-manager.cpp)
target_link_libraries(TestSmsManager PRIVATE MightyGardenerLib GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(TestSmsUtils)
gtest_discover_tests(TestSmsManager)
