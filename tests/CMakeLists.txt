project(dukglue_test)

cmake_minimum_required(VERSION 3.1.0)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../cmake_modules)

find_package(Duktape REQUIRED)
find_package(Dukglue REQUIRED)

add_executable(dukglue_test
  main.cpp
  test_assert.cpp
  test_classes.cpp
  test_inheritance.cpp
  test_multiple_contexts.cpp
  test_primitives.cpp
)

include_directories(${DUKTAPE_INCLUDE_DIR})
include_directories(${DUKGLUE_INCLUDE_DIR})
target_link_libraries(dukglue_test ${DUKTAPE_LIBRARY})

target_compile_features(dukglue_test PRIVATE cxx_variadic_templates cxx_auto_type)
