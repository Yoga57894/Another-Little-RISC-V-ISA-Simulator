cmake_minimum_required(VERSION 2.8)

project(ALISS)

find_package(GTest REQUIRED)
set(CMAKE_CXX_STANDARD 11)


set(SOURCE_FILES src/main.cxx)

set(TEST_FILES src/test_main.cxx)

add_executable(ALISS ${SOURCE_FILES})
add_executable(ALISS_TEST ${TEST_FILES})
target_link_libraries(ALISS_TEST gtest GTest::GTest GTest::Main)

add_test(MAIN_TEST ALISS_TEST)
