# - Find Miles SDK
# Find the Miles SDK header + import library
#
#  MilesSDK_INCLUDE_DIR - Where to find mss.h
#  MilesSDK_LIBRARIES   - List of libraries when using MilesSDK.
#  MilesSDK_FOUND       - True if Miles SDK found.
#  MilesSDK::MilesSDK   - Imported library of Miles SDK

find_path(MilesSDK_INCLUDE_DIR mss.h
    PATHS "${MilesSDK_DIR}"
    PATH_SUFFIXES include
)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(_miles_sdk_libname mss64)
else()
    set(_miles_sdk_libname mss32)
endif()

find_library(MilesSDK_LIBRARIES NAMES ${_miles_sdk_libname}
    PATHS "${MilesSDK_DIR}"
    PATH_SUFFIXES lib
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(MilesSDK DEFAULT_MSG MilesSDK_LIBRARIES MilesSDK_INCLUDE_DIR)

if(NOT TARGET MilesSDK::MilesSDK)
    add_library(MilesSDK::MilesSDK UNKNOWN IMPORTED)
    set_target_properties(MilesSDK::MilesSDK PROPERTIES
        IMPORTED_LOCATION "${MilesSDK_LIBRARIES}"
        INTERFACE_INCLUDE_DIRECTORIES "${MilesSDK_INCLUDE_DIR}"
    )
endif()
