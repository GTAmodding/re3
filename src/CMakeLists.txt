set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

if(${RE3_AUDIO} STREQUAL "OAL")
        find_package(OpenAL REQUIRED)
        find_package(MPG123 REQUIRED)
        find_package(SndFile REQUIRED)
endif()

file(GLOB_RECURSE Sources "*.cpp" "*.h")

MACRO(HEADER_DIRECTORIES return_list)
    FILE(GLOB_RECURSE new_list *.cpp)
    SET(dir_list "animation"
            "audio"
            "control"
            "core"
            "entities"
            "extras"
            "fakerw"
            "math"
            "modelinfo"
            "objects"
            "peds"
            "render"
            "rw"
            "save"
            "skel"
            "text"
            "vehicles"
            "weapons")
    FOREACH(file_path ${new_list})
        GET_FILENAME_COMPONENT(dir_path ${file_path} PATH)
        SET(dir_list ${dir_list} ${dir_path})
    ENDFOREACH()
    LIST(REMOVE_DUPLICATES dir_list)
    SET(${return_list} ${dir_list})
ENDMACRO()

HEADER_DIRECTORIES(header_list)
include_directories(${header_list})


add_executable(re3 ${Sources})
target_link_libraries(re3 librw)
target_link_libraries(re3 Threads::Threads)

if(${RE3_AUDIO} STREQUAL "OAL")
        target_link_libraries(re3 ${OPENAL_LIBRARY})
        target_link_libraries(re3 ${MPG123_LIBRARIES})
        target_link_libraries(re3 ${SNDFILE_LIBRARIES})
endif()

target_include_directories(re3
    INTERFACE
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
    )

target_compile_definitions(re3
    PRIVATE
        "$<IF:$<CONFIG:DEBUG>,DEBUG,NDEBUG>"
    PUBLIC
        "RW_${RE3_PLATFORM}"
    )

target_compile_definitions(re3 PRIVATE LIBRW=1 AUDIO_OAL=1)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    target_compile_options(re3
        PRIVATE
            "-Wall"
        )
    if (NOT RE3_PLATFORM_PS2)
        target_compile_options(re3
            PRIVATE
                "-Wextra"
                "-Wdouble-promotion"
                "-Wpedantic"
            )
    endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(re3
        PUBLIC
            /wd4996 /wd4244
        )
endif()

set_target_properties(re3
    PROPERTIES
        C_STANDARD 11
        C_EXTENSIONS OFF
        C_STANDARD_REQUIRED ON
        CXX_STANDARD 11
        CXX_EXTENSIONS OFF
        CXX_STANDARD_REQUIRED ON
        PREFIX ""
    )

if(RE3_INSTALL)
    target_include_directories(re3
        INTERFACE
            $<INSTALL_INTERFACE:${RE3_INSTALL_INCLUDEDIR}>
        )

    install(
        TARGETS re3
        EXPORT re3-targets
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        )
endif()
