cmake_minimum_required(VERSION 3.12)

set(
    SOURCES
    scp.cpp
    vcs.cpp
)

add_library(scp ${SOURCES})

target_include_directories(scp PUBLIC ${PROJECT_SOURCE_DIR}/src)
target_link_libraries(scp lib_posix)
add_dependencies(scp lib_posix)
