# - Find mpg123
# Find the native mpg123 includes and library
#
#  mpg123_INCLUDE_DIR - Where to find mpg123.h
#  mpg123_LIBRARIES   - List of libraries when using mpg123.
#  mpg123_CFLAGS      - Compile options to use mpg123
#  mpg123_FOUND       - True if mpg123 found.
#  MPG123::libmpg123  - Imported library of libmpg123

find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
    pkg_search_module(PKG_MPG123 mpg123)
endif()

find_path(mpg123_INCLUDE_DIR mpg123.h
    HINTS ${PKG_MPG123_INCLUDE_DIRS}
    PATHS "${mpg123_DIR}"
    PATH_SUFFIXES include
)

find_library(mpg123_LIBRARIES NAMES mpg123 mpg123-0 libmpg123-0
    HINTS ${PKG_MPG123_LIBRARIES}
    PATHS "${mpg123_DIR}"
    PATH_SUFFIXES lib
)

set(mpg123_CFLAGS "${PKG_MPG123_CFLAGS_OTHER}" CACHE STRING "CFLAGS of mpg123")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(mpg123 DEFAULT_MSG mpg123_LIBRARIES mpg123_INCLUDE_DIR)

if(NOT TARGET MPG123::libmpg123)
    add_library(__libmpg123 INTERFACE)
    target_compile_options(__libmpg123 INTERFACE ${mpg123_CFLAGS})
    target_include_directories(__libmpg123 INTERFACE ${mpg123_INCLUDE_DIR})
    target_link_libraries(__libmpg123 INTERFACE ${mpg123_LIBRARIES})
    add_library(MPG123::libmpg123 ALIAS __libmpg123)
endif()
