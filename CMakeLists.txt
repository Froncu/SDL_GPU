cmake_minimum_required(VERSION 3.29)
project(SDL_GPU)

set(CMAKE_CXX_STANDARD 23)

add_executable(${PROJECT_NAME}
   source/main.cpp
   source/shader/shader.cpp)

target_include_directories(${PROJECT_NAME}
   PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/source)

include(FetchContent)

set(SDL_TEST_LIBRARY OFF)
set(SDL_INSTALL OFF)
set(BUILD_SHARED_LIBS OFF)
FetchContent_Declare(SDL
   GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
   GIT_TAG release-3.2.16
   GIT_PROGRESS TRUE
   GIT_SHALLOW TRUE)

set(SDLSHADERCROSS_VENDORED ON)
FetchContent_Declare(SDL_shadercross
   GIT_REPOSITORY https://github.com/libsdl-org/SDL_shadercross.git
   GIT_TAG bae133234c28a602c30811c419c494a0eb241200 # TODO: replace with a release tag when available
   GIT_PROGRESS TRUE
   GIT_SHALLOW TRUE)

FetchContent_MakeAvailable(
   SDL
   SDL_shadercross)

target_link_libraries(${PROJECT_NAME} 
   PRIVATE SDL3::SDL3
   PRIVATE SDL3_shadercross::SDL3_shadercross)

add_custom_command(POST_BUILD TARGET ${PROJECT_NAME}
   COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
   ${CMAKE_SOURCE_DIR}/resources $<TARGET_FILE_DIR:${PROJECT_NAME}>/resources)