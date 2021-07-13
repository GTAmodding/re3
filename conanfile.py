from conans import ConanFile, CMake, tools
from conans.errors import ConanException, ConanInvalidConfiguration
import os
import shutil
import textwrap


class Re3Conan(ConanFile):
    name = "re3"
    version = "master"
    license = "???"  # FIXME: https://github.com/GTAmodding/re3/issues/794
    settings = "os", "arch", "compiler", "build_type"
    generators = "cmake", "cmake_find_package"
    options = {
        "audio": ["openal", "miles"],
        "with_libsndfile": [True, False],
        "with_opus": [True, False],
    }
    default_options = {
        "audio": "openal",
        "with_libsndfile": False,
        "with_opus": False,
        # "libsndfile:with_external_libs": False,
        # "mpg123:flexible_resampling": False,
        # "mpg123:network": False,
        # "mpg123:icy": False,
        # "mpg123:id3v2": False,
        # "mpg123:ieeefloat": False,
        # "mpg123:layer1": False,
        # "mpg123:layer2": False,
        # "mpg123:layer3": False,
        # "mpg123:moreinfo": False,
        # "sdl2:vulkan": False,
        # "sdl2:opengl": True,
        # "sdl2:sdl2main": True,
    }
    no_copy_source = True

    @property
    def _os_is_playstation2(self):
        try:
            return self.settings.os == "Playstation2"
        except ConanException:
            return False

    def configure(self):
        if self.options.audio != "openal":
            self.options.with_libsndfile = False

    def requirements(self):
        self.requires("librw/{}".format(self.version))
        self.requires("mpg123/1.26.4")
        if self.options.audio == "openal":
            self.requires("openal/1.21.0")
        elif self.options.audio == "miles":
            self.requires("miles-sdk/{}".format(self.version))
        if self.options.with_libsndfile:
            self.requires("libsndfile/1.0.30")
        if self.options.with_opus:
            self.requires("opusfile/0.12")

    def export_sources(self):
        for d in ("cmake", "gamefiles", "src"):
            shutil.copytree(src=d, dst=os.path.join(self.export_sources_folder, d))
        self.copy("CMakeLists.txt")

    def validate(self):
        if self.options["librw"].platform == "gl3" and self.options["librw"].gl3_gfxlib != "glfw":
            raise ConanInvalidConfiguration("Only `glfw` is supported as gl3_gfxlib.")
        #if not self.options.with_opus:
        #    if not self.options["libsndfile"].with_external_libs:
        #        raise ConanInvalidConfiguration("re3 with opus support requires a libsndfile built with external libs (=ogg/flac/opus/vorbis)")

    @property
    def _re3_audio(self):
        return {
            "miles": "MSS",
            "openal": "OAL",
        }[str(self.options.audio)]

    def build(self):
        if self.source_folder == self.build_folder:
            raise Exception("cannot build with source_folder == build_folder")
        try:
            os.unlink(os.path.join(self.install_folder, "Findlibrw.cmake"))
            tools.save("FindOpenAL.cmake",
                       textwrap.dedent(
                           """
                           set(OPENAL_FOUND ON)
                           set(OPENAL_INCLUDE_DIR ${OpenAL_INCLUDE_DIRS})
                           set(OPENAL_LIBRARY ${OpenAL_LIBRARIES})
                           set(OPENAL_DEFINITIONS ${OpenAL_DEFINITIONS})
                           """), append=True)
            if self.options["librw"].platform == "gl3" and self.options["librw"].gl3_gfxlib == "glfw":
                tools.save("Findglfw3.cmake",
                           textwrap.dedent(
                               """
                               if(NOT TARGET glfw)
                                 message(STATUS "Creating glfw TARGET")
                                 add_library(glfw INTERFACE IMPORTED)
                                 set_target_properties(glfw PROPERTIES
                                    INTERFACE_LINK_LIBRARIES CONAN_PKG::glfw)
                               endif()
                               """), append=True)
            tools.save("CMakeLists.txt",
                       textwrap.dedent(
                           """
                           cmake_minimum_required(VERSION 3.0)
                           project(cmake_wrapper)
    
                           include("{}/conanbuildinfo.cmake")
                           conan_basic_setup(TARGETS NO_OUTPUT_DIRS)
    
                           add_subdirectory("{}" re3)
                           """).format(self.install_folder.replace("\\", "/"),
                                       self.source_folder.replace("\\", "/")))
        except FileNotFoundError:
            pass
        cmake = CMake(self)
        cmake.definitions["RE3_AUDIO"] = self._re3_audio
        cmake.definitions["RE3_WITH_OPUS"] = self.options.with_opus
        cmake.definitions["RE3_INSTALL"] = True
        cmake.definitions["RE3_VENDORED_LIBRW"] = False
        env = {}
        if self._os_is_playstation2:
            cmake.definitions["CMAKE_TOOLCHAIN_FILE"] = self.deps_user_info["ps2dev-cmaketoolchain"].cmake_toolchain_file
            env["PS2SDK"] = self.deps_cpp_info["ps2dev-ps2sdk"].rootpath

        with tools.environment_append(env):
            cmake.configure(source_folder=self.build_folder)
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()
