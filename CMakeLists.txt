cmake_minimum_required(VERSION 3.1.0)
project(dukglue)

# Options
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")
    set(DUKGLUE_IS_MAIN TRUE)
else()
    set(DUKGLUE_IS_MAIN FALSE)
endif()
option(DUKGLUE_BUILD_TESTS "Tests" ${DUKGLUE_IS_MAIN})

# Register the cmake_modules folder
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/../cmake_modules)

# Includes
add_subdirectory(include)

# Tests
if(DUKGLUE_BUILD_TESTS)
  add_subdirectory(tests)
endif()

if(WIN32)
  set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/install)
endif()