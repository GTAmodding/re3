set(EXECUTABLE re3)
set(STEAMID 12100)

cmake_minimum_required(VERSION 3.8)
project(${EXECUTABLE} C CXX)
set(PROJECT_AUTHOR "GTAModding")
set(PROJECT_URL "https://github.com/gtamodding/re3")

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

if(WIN32)
    set(RE3_AUDIOS "OAL" "MSS")
else()
    set(RE3_AUDIOS "OAL")
endif()

set(RE3_AUDIO "OAL" CACHE STRING "Audio")

include(CMakeDependentOption)

option(RE3_INSTALL "Install ${EXECUTABLE}" OFF)
option(RE3_WITH_OPUS "Build ${EXECUTABLE} with opus support" OFF)
option(RE3_WITH_LIBSNDFILE "Build ${EXECUTABLE} with libsndfile (instead of internal decoder)" OFF)

cmake_dependent_option(RE3_INSTALL_NSIS "Create NSIS installer" ON "WIN32;RE3_INSTALL" OFF)
cmake_dependent_option(RE3_INSTALL_SRC "Create sources archive" OFF "RE3_INSTALL" OFF)

if(RE3_INSTALL_NSIS)
    find_program(MAKENSIS_BIN makensis)
    if(NOT MAKENSIS_BIN)
        message(FATAL_ERROR "Cannot find `makensis` for creating NSIS installer.")
    endif()
endif()

set_property(CACHE RE3_AUDIO PROPERTY STRINGS ${RE3_AUDIOS})
message(STATUS "RE3_AUDIO = ${RE3_AUDIO} (choices=${RE3_AUDIOS})")
set("RE3_AUDIO_${RE3_AUDIO}" ON)
if(NOT RE3_AUDIO IN_LIST RE3_AUDIOS)
    message(FATAL_ERROR "Illegal RE3_AUDIO=${RE3_AUDIO}")
endif()

option(RE3_VENDORED_LIBRW "Use vendored librw" ON)
if(RE3_VENDORED_LIBRW)
    add_subdirectory(vendor/librw)
else()
    find_package(librw REQUIRED)
endif()
add_subdirectory(src)
set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT ${EXECUTABLE})

if(RE3_INSTALL)
    install(DIRECTORY gamefiles/ DESTINATION "." COMPONENT comp2_gamedata)
    configure_file(cmake/CPackProjectConfig.cmake.in CPackProjectConfig.cmake @ONLY)
    #file(GENERATE OUTPUT CPackProjectConfig.cmake INPUT "${CMAKE_CURRENT_SOURCE_DIR}/cmake/CPackProjectConfig.cmake.in" TARGET ${EXECUTABLE})
    set(CPACK_PACKAGE_DIRECTORY "${CMAKE_BINARY_DIR}/dist")

    if(RE3_INSTALL_SRC)
        install(
            DIRECTORY cmake gamefiles resources src utils
            DESTINATION .
            COMPONENT compZ_sources
            EXCLUDE_FROM_ALL
        )
        install(
            FILES conanfile.py CMakeLists.txt premake5.lua README.md CODING_STYLE.md
            DESTINATION .
            COMPONENT compZ_sources
            EXCLUDE_FROM_ALL
        )
    endif()

    set(CPACK_PACKAGE_INSTALL_DIRECTORY "${PROJECT_NAME}")
    set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "GTA III reversed")
    set(CPACK_PACKAGE_VENDOR "${PROJECT_AUTHOR}")
    set(CPACK_PROJECT_CONFIG_FILE "${CMAKE_CURRENT_BINARY_DIR}/CPackProjectConfig.cmake")
    # FIXME: missing license (https://github.com/GTAmodding/re3/issues/794)
    # set(CPACK_PACKAGE_DESCRIPTION_FILE "${PROJECT_SOURCE_DIR}/LICENSE")
    # set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/LICENSE")
    set(CPACK_GENERATOR "TXZ")
    # FIXME: this is displayed in the GUI installer!!! Needs a disclaimer about trademarks etc.
    # set(CPACK_RESOURCE_FILE_LICENSE)
    set(CPACK_PACKAGE_ICON "${PROJECT_SOURCE_DIR}\\\\resources\\\\logo.bmp")
    set(CPACK_PACKAGE_CHECKSUM "SHA256")

    list(REMOVE_ITEM CPACK_COMPONENTS_ALL "compZ_sources")

    set(CPACK_ARCHIVE_COMPONENT_INSTALL ON)

    if(WIN32)
        set(NEED_RUNTIME OFF)
        if(MSVC)
            if(NOT "-MT" MATCHES CMAKE_C_FLAGS)
                # Install MSVC runtime when using dynamic runtime (-MD, -MDd)
                if("-MDd" MATCHES CMAKE_C_FLAGS)
                    set(CMAKE_INSTALL_DEBUG_LIBRARIES_ONLY ON)
                endif()
                set(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION ".")
                set(CMAKE_INSTALL_SYSTEM_RUNTIME_COMPONENT comp3_runtime)
                include(InstallRequiredSystemLibraries)
                set(NEED_RUNTIME ON)
            endif()
        endif()
        if(RE3_INSTALL_NSIS)
            list(APPEND CPACK_GENERATOR "NSIS")
        endif()

        set(CPACK_NSIS_COMPONENT_INSTALL ON)
        set(CPACK_NSIS_DEFINES "!RequestExecutionLevel user")
        set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
        set(CPACK_NSIS_DISPLAY_NAME "${PROJECT_NAME} (${PROJECT_AUTHOR})")
        set(CPACK_NSIS_HELP_LINK "${PROJECT_URL}")
        set(CPACK_NSIS_URL_INFO_ABOUT "${PROJECT_URL}")
        set(CPACK_NSIS_MUI_ICON "${PROJECT_SOURCE_DIR}\\\\resources\\\\logo.ico")
        set(CPACK_NSIS_MUI_UNIICON "${PROJECT_SOURCE_DIR}\\\\resources\\\\logo.ico")
        set(CPACK_NSIS_EXECUTABLES_DIRECTORY ".")
        set(CPACK_NSIS_MUI_FINISHPAGE_RUN "${EXECUTABLE}")
    endif()

    include(CPackComponent)

    cpack_add_component(comp1_core
        DISPLAY_NAME "Game executable(s)"
        DESCRIPTION "Install the game executable(s)."
        REQUIRED
    )
    cpack_add_component(comp2_gamedata
        DISPLAY_NAME "Game data fixes"
        DESCRIPTION "Install game data fixes."
        REQUIRED
        DEPENDS "comp1_core"
    )
    cpack_add_component(comp3_runtime
        DISPLAY_NAME "System runtime"
        DESCRIPTION "Install C runtime. Not required if a suitable MSVC runtime is already available."
        DEPENDS "comp1_core"
    )
    cpack_add_component(comp4_debug
        DISPLAY_NAME "Debug symbols"
        DESCRIPTION "Enables debugging your problems. Hopefully not needed."
        DEPENDS "comp1_core"
        DISABLED
    )
    cpack_add_component(compZ_sources
        DISPLAY_NAME "sources"
        DESCRIPTION "The sources of ${PROJECT_NAME}."
        DISABLED
        HIDDEN
    )

    include(CPack)
endif()
