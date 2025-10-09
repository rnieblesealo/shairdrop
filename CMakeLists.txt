# Adapted from Niko

cmake_minimum_required(VERSION 3.10)

project(shairdrop)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE Debug)

find_package(raylib REQUIRED)

add_executable(${PROJECT_NAME} 
  src/main.c
  src/sendhelp.c
)

if(APPLE)
    target_link_libraries(${PROJECT_NAME} raylib "-framework CoreVideo" "-framework IOKit" "-framework Cocoa" "-framework GLUT" "-framework OpenGL")
else()
    target_link_libraries(${PROJECT_NAME} raylib)
endif()

target_include_directories(shairdrop PRIVATE 
  include
)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})
