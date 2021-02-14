# Found on http://hg.kvats.net
#
# - Try to find libsndfile
# 
# Once done this will define
#
#  SNDFILE_FOUND        - system has libsndfile
#  SNDFILE_INCLUDE_DIRS - the libsndfile include directory
#  SNDFILE_LIBRARIES    - Link these to use libsndfile
#  SNDFILE_CFLAGS       - Compile options to use libsndfile
#  SndFile::SndFile     - Imported library of libsndfile
#
#  Copyright (C) 2006  Wengo
#
#  Redistribution and use is allowed according to the terms of the New
#  BSD license.
#  For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
    pkg_search_module(PKG_SNDFILE "sndfile")
endif()

find_path(SNDFILE_INCLUDE_DIR
    NAMES
        sndfile.h
    HINTS
        ${PKG_SNDFILE_INCLUDE_DIRS}
    PATHS
        /usr/include
        /usr/local/include
        /opt/local/include
        /sw/include
    )

find_library(SNDFILE_LIBRARY
    NAMES
        sndfile
    HINTS
        ${PKG_SNDFILE_LIBRARIES}
    PATHS
        /usr/lib
        /usr/local/lib
        /opt/local/lib
        /sw/lib
)

set(SNDFILE_CFLAGS "${PKG_SNDFILE_CFLAGS_OTHER}" CACHE STRING "CFLAGS of libsndfile")

set(SNDFILE_INCLUDE_DIRS "${SNDFILE_INCLUDE_DIR}")
set(SNDFILE_LIBRARIES "${SNDFILE_LIBRARY}")

if(SNDFILE_INCLUDE_DIRS AND SNDFILE_LIBRARIES)
    set(SNDFILE_FOUND TRUE)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SndFile DEFAULT_MSG SNDFILE_INCLUDE_DIRS SNDFILE_LIBRARIES)

if(NOT TARGET SndFile::SndFile)
    add_library(__SndFile INTERFACE)
    target_compile_options(__SndFile INTERFACE ${SNDFILE_CFLAGS})
    target_include_directories(__SndFile INTERFACE ${SNDFILE_INCLUDE_DIRS})
    target_link_libraries(__SndFile INTERFACE ${SNDFILE_LIBRARIES})
    add_library(SndFile::SndFile ALIAS __SndFile)
endif()
