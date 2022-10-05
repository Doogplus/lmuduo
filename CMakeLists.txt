cmake_minimum_required(VERSION 2.8)
project(lmuduo)

set(CMAKE_CXX_STANDARD 11)
set(WORKSPACE_DIR /home/$ENV{USER}/Code/lmuduo)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11 -fPIC")

aux_source_directory(. SRC_LIST)
add_library(lmuduo SHARED ${SRC_LIST} export.cpp)

add_executable(main export.cpp)