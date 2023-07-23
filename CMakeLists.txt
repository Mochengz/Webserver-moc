cmake_minimum_required(VERSION 3.0)
project(WebServer CXX)

set(CMAKE_BUILD_TYPE "Debug")

set(CXX_FLAGS
    -g
    -Wall
    -std=c++11
    -D_PTHREADS
    -pthread
    -Wno-unused-parameter
)


include_directories(${PROJECT_SOURCE_DIR}/src/include)
include_directories(${PROJECT_SOURCE_DIR}/src/base/include)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)


add_subdirectory(src)