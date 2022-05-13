cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

project(lauradb)

add_executable(${PROJECT_NAME} src/lauradb.cpp)
