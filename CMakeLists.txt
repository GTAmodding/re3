cmake_minimum_required(VERSION 3.8)

project(re3 C CXX)
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

if(WIN32)
    set(RE3_AUDIOS "NULL" "OAL" "MSS")
else()
    set(RE3_AUDIOS "NULL" "OAL")
endif()

set(RE3_AUDIO "OAL" CACHE STRING "Audio")

set_property(CACHE RE3_AUDIO PROPERTY STRINGS ${RE3_AUDIOS})
message(STATUS "RE3_AUDIO = ${RE3_AUDIO} (choices=${RE3_AUDIOS})")
set("RE3_AUDIO_${RE3_AUDIO}" ON)
if(NOT RE3_AUDIO IN_LIST RE3_AUDIOS)
    message(FATAL_ERROR "Illegal RE3_AUDIO=${RE3_AUDIO}")
endif()

if(RE3_INSTALL)
    include(GNUInstallDirs)
    set(RE3_INSTALL_INCLUDEDIR "${CMAKE_INSTALL_INCLUDEDIR}/re3")
endif()

add_subdirectory("vendor/librw")
add_subdirectory(src)

if(RE3_INSTALL)
    include(CMakePackageConfigHelpers)
    configure_package_config_file(re3-config.cmake.in re3-config.cmake
        INSTALL_DESTINATION "${CMAKE_INSTALL_PREFIX}"
        )
    install(
        FILES "${CMAKE_CURRENT_BINARY_DIR}/re3-config.cmake"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
        )
    install(
        EXPORT re3-targets
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
        )

    include(CMakeCPack.cmake)
endif()
