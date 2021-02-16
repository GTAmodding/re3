# - Try to find opusfile
# 
# Once done this will define
#
#  OPUSFILE_FOUND        - system has opusfile
#  OPUSFILE_INCLUDE_DIRS - the opusfile include directories
#  OPUSFILE_LIBRARIES    - Link these to use opusfile
#  OPUSFILE_CFLAGS       - Compile options to use opusfile
#  opusfile::opusfile    - Imported library of opusfile
#

# FIXME: opusfile does not ship an official opusfile cmake script,
# rename this file/variables/target when/if it has.

find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
    pkg_search_module(PKG_OPUSFILE "opusfile")
endif()

find_path(OPUSFILE_INCLUDE_DIR
    NAMES
        opusfile.h
    PATH_SUFFIXES
        opusfile
    HINTS
        ${PKG_OPUSFILE_INCLUDE_DIRS}
    PATHS
        /usr/include
        /usr/local/include
        /opt/local/include
        /sw/include
    )

find_library(OPUSFILE_LIBRARY
    NAMES
        opusfile
    HINTS
        ${PKG_OPUSFILE_LIBRARIES}
    PATHS
        /usr/lib
        /usr/local/lib
        /opt/local/lib
        /sw/lib
)

set(OPUSFILE_CFLAGS "${PKG_OPUSFILE_CFLAGS_OTHER}" CACHE STRING "CFLAGS of opusfile")

set(OPUSFILE_INCLUDE_DIRS "${OPUSFILE_INCLUDE_DIR}")
set(OPUSFILE_LIBRARIES "${OPUSFILE_LIBRARY}")

if (OPUSFILE_INCLUDE_DIRS AND OPUSFILE_LIBRARIES)
set(OPUSFILE_FOUND TRUE)
endif (OPUSFILE_INCLUDE_DIRS AND OPUSFILE_LIBRARIES)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(opusfile DEFAULT_MSG OPUSFILE_INCLUDE_DIRS OPUSFILE_LIBRARIES)

if(NOT TARGET opusfile::opusfile)
    add_library(__opusfile INTERFACE)
    target_compile_options(__opusfile INTERFACE ${OPUSFILE_CFLAGS})
    target_include_directories(__opusfile INTERFACE ${OPUSFILE_INCLUDE_DIRS})
    target_link_libraries(__opusfile INTERFACE ${OPUSFILE_LIBRARIES})
    add_library(opusfile::opusfile ALIAS __opusfile)
endif()
