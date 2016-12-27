cmake_minimum_required(VERSION 2.7)

add_library(dukglue INTERFACE)

#target_include_directories(dukglue ${CMAKE_CURRENT_SOURCE_DIR})

set(DUKGLUE_HEADERS
  ${CMAKE_CURRENT_SOURCE_DIR}/dukglue/dukglue.h
  ${CMAKE_CURRENT_SOURCE_DIR}/dukglue/detail_class_proto.h
  ${CMAKE_CURRENT_SOURCE_DIR}/dukglue/detail_constructor.h
  ${CMAKE_CURRENT_SOURCE_DIR}/dukglue/detail_function.h
  ${CMAKE_CURRENT_SOURCE_DIR}/dukglue/detail_method.h
  ${CMAKE_CURRENT_SOURCE_DIR}/dukglue/detail_primitive_types.h
  ${CMAKE_CURRENT_SOURCE_DIR}/dukglue/detail_refs.h
  ${CMAKE_CURRENT_SOURCE_DIR}/dukglue/detail_stack.h
  ${CMAKE_CURRENT_SOURCE_DIR}/dukglue/detail_traits.h
  ${CMAKE_CURRENT_SOURCE_DIR}/dukglue/detail_typeinfo.h
  ${CMAKE_CURRENT_SOURCE_DIR}/dukglue/detail_types.h
  ${CMAKE_CURRENT_SOURCE_DIR}/dukglue/dukvalue.h
  ${CMAKE_CURRENT_SOURCE_DIR}/dukglue/dukexception.h
  ${CMAKE_CURRENT_SOURCE_DIR}/dukglue/register_class.h
  ${CMAKE_CURRENT_SOURCE_DIR}/dukglue/register_function.h
  ${CMAKE_CURRENT_SOURCE_DIR}/dukglue/register_property.h
  ${CMAKE_CURRENT_SOURCE_DIR}/dukglue/public_util.h
)

install(FILES
  ${DUKGLUE_HEADERS} DESTINATION include/dukglue
)

target_include_directories(dukglue INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

# so CMake generates a visual studio project...
add_custom_target(dukglue_ide SOURCES ${DUKGLUE_HEADERS})