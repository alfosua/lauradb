cmake_minimum_required(VERSION 3.22)

project(
  lauradb
  VERSION 1.0
  LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

add_executable(${PROJECT_NAME} dbms/src/lauradb.cpp)
target_link_libraries(${PROJECT_NAME} CONAN_PKG::boost CONAN_PKG::fmt)
