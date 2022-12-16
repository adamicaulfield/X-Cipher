cmake_minimum_required(VERSION 3.12)
project(testPanther VERSION 0.1 LANGUAGES CXX)

SET(TEST_PANTHER_SOURCE_FILES testPanther.cpp)
add_executable(testPanther ${TEST_PANTHER_SOURCE_FILES} ${SOURCE_FILES})
target_link_libraries(testPanther m helib ntl pthread gmp gtest)
