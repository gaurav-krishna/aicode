cmake_minimum_required(VERSION 3.16)
project(LuaJITWrapper CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

option(BUILD_EXAMPLES "Build example program" ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LUAJIT QUIET luajit luajit-5.1)

if(NOT LUAJIT_FOUND)
  message(STATUS "pkg-config could not find luajit; trying manual find")
  find_path(LUAJIT_INCLUDE_DIR lua.hpp
            HINTS /usr/include/luajit-2.1 /usr/local/include/luajit-2.1
                  /usr/include/luajit-2.0 /usr/local/include/luajit-2.0
                  /usr/include /usr/local/include)
  find_library(LUAJIT_LIBRARY NAMES luajit-5.1 luajit
               HINTS /usr/lib /usr/local/lib /usr/lib64 /usr/local/lib64)
  if(LUAJIT_INCLUDE_DIR AND LUAJIT_LIBRARY)
    set(LUAJIT_FOUND TRUE)
    set(LUAJIT_INCLUDE_DIRS ${LUAJIT_INCLUDE_DIR})
    set(LUAJIT_LIBRARIES ${LUAJIT_LIBRARY})
  endif()
endif()

if(NOT LUAJIT_FOUND)
  message(FATAL_ERROR "LuaJIT not found. Please install libluajit (dev) and pkg-config.")
endif()

message(STATUS "Using LuaJIT include dirs: ${LUAJIT_INCLUDE_DIRS}")
message(STATUS "Using LuaJIT libraries: ${LUAJIT_LIBRARIES}")

add_library(luajit_wrapper STATIC
  src/LuaJITWrapper.cpp
)

target_include_directories(luajit_wrapper PUBLIC
  ${PROJECT_SOURCE_DIR}/include
  ${LUAJIT_INCLUDE_DIRS}
)

find_package(Threads)
set(EMBED_LIBS ${LUAJIT_LIBRARIES})
if(Threads_FOUND)
  list(APPEND EMBED_LIBS Threads::Threads)
endif()
find_library(DL_LIBRARY dl)
if(DL_LIBRARY)
  list(APPEND EMBED_LIBS ${DL_LIBRARY})
endif()

target_link_libraries(luajit_wrapper PUBLIC ${EMBED_LIBS})

if(BUILD_EXAMPLES)
  add_executable(example_embed examples/main.cpp)
  target_link_libraries(example_embed PRIVATE luajit_wrapper)
  target_include_directories(example_embed PRIVATE ${PROJECT_SOURCE_DIR}/include)
  add_custom_command(TARGET example_embed POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${PROJECT_SOURCE_DIR}/scripts $<TARGET_FILE_DIR:example_embed>/scripts)
endif()