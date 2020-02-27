cmake_minimum_required(VERSION 3.12)
project(scpcopier)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(koloshmet_lib)
add_subdirectory(src)

add_compile_definitions(SCPCOPIER_VERSION="1.0.0")

add_executable(scpcopier src/main.cpp)

target_link_libraries(scpcopier scp)
