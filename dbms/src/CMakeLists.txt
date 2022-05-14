set(DBMS_BINARY ${PROJECT_NAME})

file(GLOB_RECURSE SOURCES LIST_DIRECTORIES true *.hpp *.cpp)

add_executable(${DBMS_BINARY} ${SOURCES})
add_library(${DBMS_BINARY}_lib STATIC ${SOURCES})

target_link_libraries(${DBMS_BINARY} CONAN_PKG::boost CONAN_PKG::fmt)
