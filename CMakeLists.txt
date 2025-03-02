cmake_minimum_required(VERSION 3.20)

#
project(Swoosh
  VERSION 2.0.0
  DESCRIPTION "Screen Transition Management Library Currently only for SFML."
  LANGUAGES CXX
)
set (PROJECT_AUTHOR "TheMaverickProgrammer")

include (FetchContent)
FetchContent_Declare (SFML
  GIT_REPOSITORY https://github.com/SFML/SFML.git
  GIT_TAG 3.0.0
  GIT_SHALLOW ON
  EXCLUDE_FROM_ALL
  SYSTEM
)
FetchContent_MakeAvailable (SFML)

# Set Swoosh library
add_library(${PROJECT_NAME} INTERFACE)
add_library(Swoosh::${PROJECT_NAME} ALIAS Swoosh)

target_include_directories(${PROJECT_NAME} INTERFACE "${PROJECT_SOURCE_DIR}/src")

# Build a library so that swoosh can be added like SFML
set (LIBRARY_OUTPUT_PATH "${PROJECT_BINARY_DIR}/lib")

# Configuration
target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_17)
# include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/Config.cmake)

# Remove use of glob and build library
add_subdirectory (src)

# Build demo
add_executable(Demo ExampleDemo/Demo.cpp)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

target_link_libraries(Demo PRIVATE
  SFML::Graphics
  SFML::Audio
  SFML::Network
  Swoosh
)

set_target_properties(Demo
  PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/build/$<CONFIG>"
)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/Compiler.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/PostBuild.cmake)
