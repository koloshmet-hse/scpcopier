set(CMAKE_CXX_STANDARD 17)

add_compile_options(-Werror -Wall -Wextra)
if (CMAKE_BUILD_TYPE MATCHES Debug)
    add_compile_options(-g)
elseif (CMAKE_BUILD_TYPE MATCHES Release)
    add_compile_options(-O3)
endif()

add_compile_definitions(SCPCOPIER_VERSION="${PROJECT_VERSION}")

set(
    SOURCES
    scp.cpp
    vcs.cpp
    config.cpp
    options.cpp
)

add_library(scp ${SOURCES})

target_include_directories(scp PUBLIC ${PROJECT_SOURCE_DIR}/src)
target_link_libraries(scp k_posix)
add_dependencies(scp k_posix)

add_executable(scpcopier main.cpp)
target_link_libraries(scpcopier scp)
add_dependencies(scpcopier scp)
