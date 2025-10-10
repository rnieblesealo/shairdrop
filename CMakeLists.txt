# Adapted from Niko
# TODO: Make sure this isn't horrible :)

cmake_minimum_required(VERSION 3.10)

project(shairdrop)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE Debug)

find_package(raylib REQUIRED)

add_executable(shairc 
  src/client.c
  src/shairdrop.c
)

add_executable(shairs
  src/server.c
  src/sendhelp.c
  src/shairdrop.c
)

if(APPLE)
    target_link_libraries(shairc raylib "-framework CoreVideo" "-framework IOKit" "-framework Cocoa" "-framework GLUT" "-framework OpenGL")
    target_link_libraries(shairs raylib "-framework CoreVideo" "-framework IOKit" "-framework Cocoa" "-framework GLUT" "-framework OpenGL")
else()
    target_link_libraries(shairc raylib)
    target_link_libraries(shairs raylib)
endif()

target_include_directories(shairc PRIVATE 
  include
)

target_include_directories(shairs PRIVATE 
  include
)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})
