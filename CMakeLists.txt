cmake_minimum_required(VERSION 3.14)
project(scpcopier)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(koloshmet_lib)

set(
    SOURCES
    src/scp.cpp
)

add_library(scp ${SOURCES})

target_include_directories(scp PUBLIC ${PROJECT_SOURCE_DIR}/src)
target_link_libraries(scp lib_posix)

add_executable(scpcopier src/main.cpp)

target_link_libraries(scpcopier scp)
add_dependencies(scpcopier scp)
