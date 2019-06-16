workspace "re3"
	configurations { "DebugCI", "ReleaseCI", "Release", "ReleaseFH", "Debug" }
	location "build"

	files { "src/*.*" }
	files { "src/skel/*.*" }
	files { "src/skel/win/*.*" }
	files { "src/math/*.*" }
	files { "src/modelinfo/*.*" }
	files { "src/entities/*.*" }
	files { "src/weapons/*.*" }
	files { "src/render/*.*" }
	files { "src/control/*.*" }
	files { "src/animation/*.*" }
	files { "src/audio/*.*" }

	includedirs { "src" }
	includedirs { "src/skel/" }
	includedirs { "src/skel/win" }
	includedirs { "src/modelinfo" }
	includedirs { "src/entities" }
	includedirs { "src/weapons" }
	includedirs { "src/render" }
	includedirs { "src/control" }
	includedirs { "src/audio" }
	includedirs { "src/animation" }
	includedirs { "dxsdk/include" }
	includedirs { "rwsdk/include/d3d8" }
	
	libdirs { "dxsdk/lib" }

project "re3"
	kind "SharedLib"
	language "C++"
	targetname "re3"
	targetdir "bin/%{cfg.buildcfg}"
	targetextension ".dll"
	characterset ("MBCS")
	linkoptions "/SAFESEH:NO"

	filter "configurations:Debug"
		defines { "DEBUG" }
		staticruntime "on"
		symbols "On"
		debugdir "$(GTA_III_RE_DIR)"
		debugcommand "$(GTA_III_RE_DIR)/gta3.exe"
		postbuildcommands "copy /y \"$(TargetPath)\" \"$(GTA_III_RE_DIR)\\plugins\\re3.dll\""

	filter "configurations:Release"
		defines { "NDEBUG" }
		optimize "On"
		staticruntime "on"
		debugdir "C:/Users/aap/games/gta3_re"
		debugcommand "C:/Users/aap/games/gta3_re/gta3.exe"
		postbuildcommands "copy /y \"$(TargetPath)\" \"$(GTA_III_RE_DIR)\\plugins\\re3.dll\""
	filter "configurations:ReleaseFH"
		defines { "NDEBUG" }
		symbols "Full"
		optimize "off"
		staticruntime "on"
		debugdir "$(GTA_III_DIR)"
		debugcommand "$(GTA_III_DIR)/gta3.exe"
		targetextension ".asi"
		targetdir "$(GTA_III_DIR)/scripts"
	filter "configurations:DebugCI"
		defines { "DEBUG" }
		symbols "On"
		staticruntime "on"
	filter "configurations:ReleaseCI"
		defines { "NDEBUG" }
		optimize "On"
		staticruntime "on"
