cmake_minimum_required(VERSION 2.7)

project(duktape)

set(CMAKE_DEBUG_POSTFIX "d")

add_library(duktape SHARED
  duktape.c
  duktape.h
  duk_config.h
)

install(TARGETS duktape
  LIBRARY DESTINATION lib
)

install(FILES
  duktape.h
  duk_config.h
  DESTINATION include
)
