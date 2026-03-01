cmake_minimum_required(VERSION 3.29)
project(SDL_GPU)

file(GLOB_RECURSE SOURCES source/*.cpp)
add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME}
   PRIVATE source)

set_target_properties(${PROJECT_NAME} PROPERTIES
   CXX_STANDARD 23
   CXX_STANDARD_REQUIRED ON)

target_compile_options(${PROJECT_NAME}
   PRIVATE $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic -Werror>
   PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>)

find_package(SDL3 CONFIG REQUIRED COMPONENTS SDL3-static)
find_package(SDL3_shadercross CONFIG REQUIRED COMPONENTS SDL3_shadercross-static)
find_package(glm CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} 
   PRIVATE SDL3::SDL3
   PRIVATE SDL3_shadercross::SDL3_shadercross
   PRIVATE glm::glm-header-only)

add_custom_command(POST_BUILD TARGET ${PROJECT_NAME}
   COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
   ${CMAKE_SOURCE_DIR}/resources $<TARGET_FILE_DIR:${PROJECT_NAME}>/resources)

if(EMSCRIPTEN)
   set(EMSCRIPTEN_WEB_DIRECTORY ${CMAKE_SOURCE_DIR}/web)
   set(EMSCRIPTEN_SHELL_FILE ${EMSCRIPTEN_WEB_DIRECTORY}/shell.html)

   set_target_properties(${PROJECT_NAME} PROPERTIES
      SUFFIX ".html"
      LINK_DEPENDS ${EMSCRIPTEN_SHELL_FILE})

   target_compile_options(${PROJECT_NAME}
      PRIVATE -pthread)

   target_link_libraries(${PROJECT_NAME}
      PRIVATE -sASSERTIONS
      PRIVATE -sUSE_PTHREADS
      PRIVATE -sPTHREAD_POOL_SIZE=2
      PRIVATE -sALLOW_MEMORY_GROWTH
      PRIVATE --shell-file=${EMSCRIPTEN_SHELL_FILE})

   add_custom_target(${PROJECT_NAME}_host_locally
      DEPENDS ${PROJECT_NAME}
      USES_TERMINAL
      COMMAND node server.js -d ${CMAKE_BINARY_DIR} -f ${PROJECT_NAME}.html
      WORKING_DIRECTORY ${EMSCRIPTEN_WEB_DIRECTORY})
endif()