cmake_minimum_required(VERSION 3.0)
project(RC)

set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -g)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/thirdparty)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/build)

add_subdirectory(src)