Librw = os.getenv("LIBRW") or "librw"

workspace "re3"
	configurations { "Debug", "Release", "ReleaseFH", "DebugRW", "ReleaseRW"  }
	location "build"

	files { "src/*.*" }
	files { "src/animation/*.*" }
	files { "src/audio/*.*" }
	files { "src/control/*.*" }
	files { "src/core/*.*" }
	files { "src/entities/*.*" }
	files { "src/math/*.*" }
	files { "src/modelinfo/*.*" }
	files { "src/objects/*.*" }
	files { "src/peds/*.*" }
	files { "src/render/*.*" }
	files { "src/rw/*.*" }
	files { "src/save/*.*" }
	files { "src/skel/*.*" }
	files { "src/skel/win/*.*" }
	files { "src/text/*.*" }
	files { "src/vehicles/*.*" }
	files { "src/weapons/*.*" }
	files { "src/stuff/*.*" }
	files { "eax/*.*" }

	includedirs { "src" }
	includedirs { "src/animation" }
	includedirs { "src/audio" }
	includedirs { "src/control" }
	includedirs { "src/core" }
	includedirs { "src/entities" }
	includedirs { "src/math" }
	includedirs { "src/modelinfo" }
	includedirs { "src/objects" }
	includedirs { "src/peds" }
	includedirs { "src/render" }
	includedirs { "src/rw" }
	includedirs { "src/save/" }
	includedirs { "src/skel/" }
	includedirs { "src/skel/win" }
	includedirs { "src/text" }
	includedirs { "src/vehicles" }
	includedirs { "src/weapons" }
	includedirs { "src/stuff" }
	includedirs { "eax" }

	includedirs { "dxsdk/include" }
	includedirs { "milessdk/include" }
	includedirs { "eax" }

	libdirs { "dxsdk/lib" }
	libdirs { "milessdk/lib" }

	filter "configurations:Debug or Release"
		files { "src/fakerw/*.*" }
		includedirs { "src/fakerw" }
		includedirs { Librw }
		libdirs { path.join(Librw, "lib/win-x86-d3d9/%{cfg.buildcfg}") }
		links { "rw", "d3d9" }
	filter  {}
	
	filter "configurations:DebugRW or ReleaseRW"
		defines { "RWLIBS" }
		includedirs { "rwsdk/include/d3d8" }
		libdirs { "rwsdk/lib/d3d8/release" }
		links { "rwcore", "rpworld", "rpmatfx", "rpskin", "rphanim", "rtbmp" }
	filter  {}

	
    pbcommands = { 
       "setlocal EnableDelayedExpansion",
       "set file=$(TargetPath)",
       "FOR %%i IN (\"%file%\") DO (",
       "set filename=%%~ni",
       "set fileextension=%%~xi",
       "set target=!path!!filename!!fileextension!",
       "if exist \"!target!\" copy /y \"!file!\" \"!target!\"",
       ")" }
    
    function setpaths (gamepath, exepath, scriptspath)
       scriptspath = scriptspath or ""
       if (gamepath) then
          cmdcopy = { "set \"path=" .. gamepath .. scriptspath .. "\"" }
          table.insert(cmdcopy, pbcommands)
          postbuildcommands (cmdcopy)
          debugdir (gamepath)
          if (exepath) then
             debugcommand (gamepath .. exepath)
             dir, file = exepath:match'(.*/)(.*)'
             debugdir (gamepath .. (dir or ""))
          end
       end
       --targetdir ("bin/%{prj.name}/" .. scriptspath)
    end

project "re3"
	kind "WindowedApp"
	language "C++"
	targetname "re3"
	targetdir "bin/%{cfg.buildcfg}"
	targetextension ".exe"
	characterset ("MBCS")
	linkoptions "/SAFESEH:NO"

	filter "configurations:Debug"
		defines { "DEBUG", "LIBRW", "RW_D3D9" }
		staticruntime "off"
		symbols "Full"
		setpaths("$(GTA_III_RE_DIR)/", "$(TargetFileName)", "")

	filter "configurations:Release"
		defines { "NDEBUG", "LIBRW", "RW_D3D9" }
		optimize "On"
		staticruntime "off"
		symbols "Full"
		setpaths("$(GTA_III_RE_DIR)/", "$(TargetFileName)", "")
		
	filter "configurations:ReleaseFH"
		defines { "NDEBUG" }
		symbols "Full"
		optimize "off"
		staticruntime "on"
		setpaths("$(GTA_III_RE_DIR)/", "$(TargetFileName)", "")

	filter "configurations:DebugRW"
		defines { "DEBUG" }
		staticruntime "on"
		symbols "On"
		setpaths("$(GTA_III_RE_DIR)/", "$(TargetFileName)", "")
		linkoptions "/SECTION:_rwcseg,ER!W /MERGE:_rwcseg=.text"

	filter "configurations:ReleaseRW"
		defines { "NDEBUG" }
		optimize "On"
		staticruntime "on"
		setpaths("$(GTA_III_RE_DIR)/", "$(TargetFileName)", "")
		linkoptions "/SECTION:_rwcseg,ER!W /MERGE:_rwcseg=.text"

