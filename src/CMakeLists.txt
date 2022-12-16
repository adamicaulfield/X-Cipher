cmake_minimum_required(VERSION 3.15)
project(${Project_Name} LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

add_executable(${Project_Name} panther.cpp ${SOURCE_FILES})
target_link_libraries(${Project_Name} PRIVATE ${OpenMP_CXX_FLAGS} m helib ntl pthread gmp)
target_compile_options(${Project_Name} PRIVATE ${OpenMP_CXX_FLAGS})
