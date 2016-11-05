cmake_minimum_required(VERSION 2.7)

project(dukglue)

add_library(dukglue INTERFACE)

install(FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/dukglue.h
  ${CMAKE_CURRENT_SOURCE_DIR}/detail_class_proto.h
  ${CMAKE_CURRENT_SOURCE_DIR}/detail_constructor.h
  ${CMAKE_CURRENT_SOURCE_DIR}/detail_function.h
  ${CMAKE_CURRENT_SOURCE_DIR}/detail_method.h
  ${CMAKE_CURRENT_SOURCE_DIR}/detail_primitive_types.h
  ${CMAKE_CURRENT_SOURCE_DIR}/detail_refs.h
  ${CMAKE_CURRENT_SOURCE_DIR}/detail_stack.h
  ${CMAKE_CURRENT_SOURCE_DIR}/detail_traits.h
  ${CMAKE_CURRENT_SOURCE_DIR}/detail_typeinfo.h
  ${CMAKE_CURRENT_SOURCE_DIR}/detail_types.h
  ${CMAKE_CURRENT_SOURCE_DIR}/register_class.h
  ${CMAKE_CURRENT_SOURCE_DIR}/register_function.h
  ${CMAKE_CURRENT_SOURCE_DIR}/register_property.h

  DESTINATION include/dukglue
)
