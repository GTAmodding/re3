set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

file(GLOB_RECURSE RE3_SOURCES "*.cpp" "*.h" "*.rc")

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${RE3_SOURCES})

function(header_directories RETURN_LIST)
    file(GLOB_RECURSE ALL_SRCS *.h *.cpp *.c)
    set(RELDIRS)
    foreach(SRC ${ALL_SRCS})
        file(RELATIVE_PATH RELSRC "${CMAKE_CURRENT_SOURCE_DIR}" "${SRC}")
        get_filename_component(RELDIR "${RELSRC}" DIRECTORY)
        list(APPEND RELDIRS ${RELDIR})
    endforeach()
    list(REMOVE_DUPLICATES RELDIRS)
    set(${RETURN_LIST} ${RELDIRS} PARENT_SCOPE)
endfunction()

header_directories(RE3_INCLUDES)

add_executable(${EXECUTABLE} WIN32
    ${RE3_SOURCES}
)

target_link_libraries(${EXECUTABLE} PRIVATE
    librw::librw
    Threads::Threads
)

target_include_directories(${EXECUTABLE}
    PRIVATE
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${RE3_INCLUDES}>
)

target_compile_definitions(${EXECUTABLE}
    PRIVATE
        $<IF:$<CONFIG:DEBUG>,DEBUG,NDEBUG>
        LIBRW
        RE3_NO_AUTOLINK
        RELOCATABLE
)
if(STEAMID)
    target_compile_definitions(${EXECUTABLE}
        PRIVATE
            STEAMID=${STEAMID}
    )
endif()

if(LIBRW_PLATFORM_D3D9)
    target_compile_definitions(${EXECUTABLE}
        PUBLIC
            USE_D3D9
    )
endif()

if(RE3_AUDIO STREQUAL "OAL")
    find_package(OpenAL REQUIRED)
    target_include_directories(${EXECUTABLE} PRIVATE ${OPENAL_INCLUDE_DIR})
    target_link_libraries(${EXECUTABLE} PRIVATE ${OPENAL_LIBRARY})
    target_compile_definitions(${EXECUTABLE} PRIVATE ${OPENAL_DEFINITIONS})
    target_compile_definitions(${EXECUTABLE} PRIVATE AUDIO_OAL)
elseif(RE3_AUDIO STREQUAL "MSS")
    find_package(MilesSDK REQUIRED)
    target_compile_definitions(${EXECUTABLE} PRIVATE AUDIO_MSS)
    target_link_libraries(${EXECUTABLE} PRIVATE MilesSDK::MilesSDK)
endif()

find_package(mpg123 REQUIRED)
target_link_libraries(${EXECUTABLE} PRIVATE
    MPG123::libmpg123
)
if(RE3_WITH_OPUS)
    find_package(opusfile REQUIRED)
    target_link_libraries(${EXECUTABLE} PRIVATE
        opusfile::opusfile
    )
    target_compile_definitions(${EXECUTABLE} PRIVATE AUDIO_OPUS)
endif()
if(RE3_WITH_LIBSNDFILE)
    find_package(SndFile REQUIRED)
    target_link_libraries(${EXECUTABLE} PRIVATE
        SndFile::SndFile
    )
    target_compile_definitions(${EXECUTABLE} PRIVATE AUDIO_OAL_USE_SNDFILE)
endif()

target_compile_definitions(${EXECUTABLE} PRIVATE )

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    target_compile_options(${EXECUTABLE}
        PRIVATE
            "-Wall"
    )
    if (NOT LIBRW_PLATFORM_PS2)
        target_compile_options(${EXECUTABLE}
            PRIVATE
                -Wextra
                -Wdouble-promotion
                -Wpedantic
        )
    endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(${EXECUTABLE}
        PUBLIC
            /Zc:sizedDealloc-
    )
endif()

set_target_properties(${EXECUTABLE}
    PROPERTIES
        C_STANDARD 11
        C_EXTENSIONS OFF
        C_STANDARD_REQUIRED ON
        CXX_STANDARD 11
        CXX_EXTENSIONS OFF
        CXX_STANDARD_REQUIRED ON
        VS_DEBUGGER_ENVIRONMENT "GTA3DIR=$<SHELL_PATH:${PROJECT_SOURCE_DIR}/gamefiles>"
        VS_DEBUGGER_COMMAND_ARGUMENTS "-console"
        VS_JUST_MY_CODE_DEBUGGING ON
)

if(RE3_INSTALL)
    install(
        TARGETS ${EXECUTABLE}
        EXPORT ${EXECUTABLE}-targets
        RUNTIME DESTINATION "."
        COMPONENT comp1_core
    )
    if(MSVC)
        install(
            FILES $<TARGET_PDB_FILE:${EXECUTABLE}>
            DESTINATION "." OPTIONAL
            COMPONENT comp4_debug
        )
    endif()
endif()
