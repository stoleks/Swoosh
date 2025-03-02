cmake_minimum_required(VERSION 3.20)

project(Swoosh-Demo)

set (CMAKE_CXX_STANDARD 17)

include (FetchContent)
FetchContent_Declare (SFML
  GIT_REPOSITORY https://github.com/SFML/SFML.git
  GIT_TAG 3.0.0
  GIT_SHALLOW ON
  EXCLUDE_FROM_ALL
  SYSTEM
)
FetchContent_MakeAvailable (SFML)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/Config.cmake)

file(GLOB SwooshFiles CONFIGURE_DEPENDS
        "ExampleDemo/*.h"
        "ExampleDemo/Scenes/*.h"
        "ExampleDemo/*.cpp"
        "ExampleDemp/Scenes/*.cpp"
        )

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

add_executable(Demo ExampleDemo/Demo.cpp ${SwooshFiles})

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

source_group(TREE ${PROJECT_SOURCE_DIR} FILES ${SwooshFiles})

target_link_libraries(Demo SFML::Graphics SFML::Audio SFML::Network)

set_target_properties(Demo
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/build/$<CONFIG>"
)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/Compiler.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/PostBuild.cmake)
