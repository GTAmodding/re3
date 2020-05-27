newoption {
	trigger     = "glewdir",
	value       = "PATH",
	description = "Directory of GLEW",
	default     = "glew-2.1.0"
}

newoption {
	trigger     = "glfwdir",
	value       = "PATH",
	description = "Directory of glfw",
	default     = "glfw-3.3.2.bin.WIN32"
}

newoption {
	trigger     = "with-librw",
	description = "Build and use librw from this solution"
}

if(_OPTIONS["with-librw"]) then
	Librw = "librw"
else
	Librw = os.getenv("LIBRW") or "librw"
end

function getsys(a)
	if a == 'windows' then
		return 'win'
	end
	return a
end

function getarch(a)
	if a == 'x86_64' then
		return 'amd64'
	elseif a == 'ARM' then
		return 'arm'
	end
	return a
end

workspace "reVC"
	language "C++"
	configurations { "Debug", "Release" }
	location "build"
	symbols "Full"
	staticruntime "off"

	filter { "system:windows" }
		platforms {
			"win-x86-RW33_d3d8-mss",
			"win-x86-librw_d3d9-mss",
			"win-x86-librw_gl3_glfw-mss",
			"win-x86-RW33_d3d8-oal",
			"win-x86-librw_d3d9-oal",
			"win-x86-librw_gl3_glfw-oal",
		}

	filter { "system:linux" }
		platforms {
			"linux-x86-librw_gl3_glfw-oal",
			"linux-amd64-librw_gl3_glfw-oal",
			"linux-arm-librw_gl3_glfw-oal",
		}

	filter "configurations:Debug"
		defines { "DEBUG" }
		
	filter "configurations:Release"
		defines { "NDEBUG" }
		optimize "On"

	filter { "platforms:win*" }
		system "windows"

	filter { "platforms:linux*" }
		system "linux"
		
	filter { "platforms:*x86*" }
		architecture "x86"
		
	filter { "platforms:*amd64*" }
		architecture "amd64"

	filter { "platforms:*arm*" }
		architecture "ARM"

	filter { "platforms:*librw_d3d9*" }
		defines { "RW_D3D9" }
		if(not _OPTIONS["with-librw"]) then
			libdirs { path.join(Librw, "lib/win-x86-d3d9/%{cfg.buildcfg}") }
		end
		
	filter "platforms:*librw_gl3_glfw*"
		defines { "RW_GL3" }
		includedirs { path.join(_OPTIONS["glfwdir"], "include") }
		includedirs { path.join(_OPTIONS["glewdir"], "include") }
		if(not _OPTIONS["with-librw"]) then
			libdirs { path.join(Librw, "lib/%{getsys(cfg.system)}-%{getarch(cfg.architecture)}-gl3/%{cfg.buildcfg}") }
		end

	filter "platforms:win*librw_gl3_glfw*"
		defines { "GLEW_STATIC" }

	filter  {}
		
    function setpaths (gamepath, exepath, scriptspath)
       scriptspath = scriptspath or ""
       if (gamepath) then
          postbuildcommands {
             '{COPY} "%{cfg.buildtarget.abspath}" "' .. gamepath .. scriptspath .. '%{cfg.buildtarget.name}"'
          }
          debugdir (gamepath)
          if (exepath) then
			 -- Used VS variable $(TargetFileName) because it doesn't accept premake tokens. Does debugcommand even work outside VS??
             debugcommand (gamepath .. "$(TargetFileName)")
             dir, file = exepath:match'(.*/)(.*)'
             debugdir (gamepath .. (dir or ""))
          end
       end
       --targetdir ("bin/%{prj.name}/" .. scriptspath)
    end

if(_OPTIONS["with-librw"]) then
project "librw"
	kind "StaticLib"
	targetname "rw"
	targetdir "lib/%{cfg.platform}/%{cfg.buildcfg}"
	files { path.join(Librw, "src/*.*") }
	files { path.join(Librw, "src/*/*.*") }
	filter "platforms:*RW33*"
		flags { "ExcludeFromBuild" }
	filter  {}
end

local function addSrcFiles( prefix )
	return prefix .. "/*cpp", prefix .. "/*.h", prefix .. "/*.c", prefix .. "/*.ico", prefix .. "/*.rc"
end

project "reVC"
	kind "WindowedApp"
	targetname "reVC"
	targetdir "bin/%{cfg.platform}/%{cfg.buildcfg}"
	defines { "MIAMI" }

	files { addSrcFiles("src") }
	files { addSrcFiles("src/animation") }
	files { addSrcFiles("src/audio") }
	files { addSrcFiles("src/audio/oal") }
	files { addSrcFiles("src/control") }
	files { addSrcFiles("src/core") }
	files { addSrcFiles("src/entities") }
	files { addSrcFiles("src/math") }
	files { addSrcFiles("src/modelinfo") }
	files { addSrcFiles("src/objects") }
	files { addSrcFiles("src/peds") }
	files { addSrcFiles("src/render") }
	files { addSrcFiles("src/rw") }
	files { addSrcFiles("src/save") }
	files { addSrcFiles("src/skel") }
	files { addSrcFiles("src/skel/glfw") }
	files { addSrcFiles("src/text") }
	files { addSrcFiles("src/vehicles") }
	files { addSrcFiles("src/weapons") }
	files { addSrcFiles("src/extras") }
	files { addSrcFiles("eax") }

	includedirs { "src" }
	includedirs { "src/animation" }
	includedirs { "src/audio" }
	includedirs { "src/audio/oal" }
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
	includedirs { "src/skel/glfw" }
	includedirs { "src/text" }
	includedirs { "src/vehicles" }
	includedirs { "src/weapons" }
	includedirs { "src/extras" }
	includedirs { "eax" }
	
	filter "platforms:*mss"
		defines { "AUDIO_MSS" }
		includedirs { "milessdk/include" }
		libdirs { "milessdk/lib" }
		
	filter "platforms:*oal"
		defines { "AUDIO_OAL" }

	filter {}
	if(os.getenv("GTA_VC_RE_DIR")) then
		setpaths("$(GTA_VC_RE_DIR)/", "%(cfg.buildtarget.name)", "")
	end
	
	filter "platforms:win*"
		files { addSrcFiles("src/skel/win") }
		includedirs { "src/skel/win" }
		linkoptions "/SAFESEH:NO"
		characterset ("MBCS")
		targetextension ".exe"

	filter "platforms:win*oal"
		includedirs { "openal-soft/include" }
		includedirs { "libsndfile/include" }
		includedirs { "mpg123/include" }
		libdirs { "openal-soft/libs/Win32" }
		libdirs { "libsndfile/lib" }
		libdirs { "mpg123/lib" }

	filter "platforms:linux*oal"
		links { "openal", "mpg123", "sndfile", "pthread" }

	filter "platforms:*RW33*"
		staticruntime "on"
		includedirs { "rwsdk/include/d3d8" }
		libdirs { "rwsdk/lib/d3d8/release" }
		links { "rwcore", "rpworld", "rpmatfx", "rpskin", "rphanim", "rtbmp", "rtquat", "rtcharse" }
		defines { "RWLIBS" }
		linkoptions "/SECTION:_rwcseg,ER!W /MERGE:_rwcseg=.text"
	
	filter "platforms:*librw*"
		defines { "LIBRW" }
		files { addSrcFiles("src/fakerw") }
		includedirs { "src/fakerw" }
		includedirs { Librw }
		if(_OPTIONS["with-librw"]) then
			libdirs { "lib/%{cfg.platform}/%{cfg.buildcfg}" }
		end
		links { "rw" }

	filter "platforms:*d3d*"
		includedirs { "dxsdk/include" }
		libdirs { "dxsdk/lib" }

	filter "platforms:*d3d9*"
		links { "d3d9" }
		
	filter "platforms:win*gl3_glfw*"
		libdirs { path.join(_OPTIONS["glewdir"], "lib/Release/Win32") }
		libdirs { path.join(_OPTIONS["glfwdir"], "lib-" .. string.gsub(_ACTION or '', "vs", "vc")) }
		links { "opengl32", "glew32s", "glfw3" }

	filter "platforms:linux*gl3_glfw*"
		links { "GL", "GLEW", "glfw" }
