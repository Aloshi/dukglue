cmake_minimum_required(VERSION 3.1.0)
project(dukglue)

add_subdirectory(include)
add_subdirectory(tests)

if(WIN32)
  set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/install)
endif()