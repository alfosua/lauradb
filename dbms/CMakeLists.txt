cmake_minimum_required(VERSION 3.22)

project(
  lauradb
  VERSION 1.0
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_compile_options(-Wall -Wextra -pedantic -Werror)

# set -fconcepts if compiler supports it
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  add_compile_options(-fconcepts)
endif()

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

add_subdirectory(src)
