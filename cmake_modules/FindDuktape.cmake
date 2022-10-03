# Duktape
include(FetchContent)
FetchContent_Declare(
    duktapecmake
    GIT_REPOSITORY https://github.com/robloach/duktape-cmake.git
    GIT_TAG v2.7.0
)
FetchContent_GetProperties(duktapecmake)
if (NOT duktapecmake_POPULATED)
    set(FETCHCONTENT_QUIET NO)
    FetchContent_Populate(duktapecmake)
    add_subdirectory(${duktapecmake_SOURCE_DIR} ${duktapecmake_BINARY_DIR})
endif()