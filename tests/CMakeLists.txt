cmake_minimum_required(VERSION 3.1.0)

add_executable(dukglue_test
  main.cpp
  test_assert.cpp
  test_classes.cpp
  test_inheritance.cpp
  test_multiple_contexts.cpp
  test_primitives.cpp
  test_properties.cpp
  test_dukvalue.cpp

  duktape.h
  duktape.c
  duk_config.h
)

# this is stupid
target_include_directories(dukglue_test PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include .)

target_compile_features(dukglue_test PRIVATE cxx_variadic_templates cxx_auto_type)
