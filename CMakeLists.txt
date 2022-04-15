cmake_minimum_required(VERSION 3.10.2)
project(tmxtool CXX)

set(CMAKE_CXX_STANDARD 17)
add_compile_options(-ggdb)

include(${CMAKE_SOURCE_DIR}/cotire.cmake)

link_directories(
    ${PROJECT_SOURCE_DIR}/lib
)
include_directories(
    ${PROJECT_SOURCE_DIR}/include
)
add_executable(${PROJECT_NAME} src/main.cpp)
aux_source_directory(src SRC)

target_sources(${PROJECT_NAME} PRIVATE ${SRC})
target_link_libraries(${PROJECT_NAME} 
    tinytmx 
    tinyxml2
    yaml-cpp
    zstd 
    z
)

cotire(${PROJECT_NAME})
