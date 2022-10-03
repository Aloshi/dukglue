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
)

# Duktape
find_package(Duktape)
target_link_libraries(dukglue_test PUBLIC duktape dukglue)

target_compile_features(dukglue_test PRIVATE cxx_variadic_templates cxx_auto_type)
