find_path(DUKGLUE_INCLUDE_DIR dukglue.h
  ${DUKGLUE_DIR}/dukglue
  ${DUKGLUE_DIR}
  $ENV{DUKGLUE_DIR}/dukglue
  $ENV{DUKGLUE_DIR}
  /usr/local/include/dukglue
  /usr/include/dukglue
  ${CMAKE_CURRENT_SOURCE_DIR}/../dukglue
)

