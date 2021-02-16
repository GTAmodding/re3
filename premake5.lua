newoption {
	trigger     = "glewdir",
	value       = "PATH",
	description = "Directory of GLEW",
	default     = "vendor/glew-2.1.0"
}

newoption {
	trigger     = "glfwdir64",
	value       = "PATH",
	description = "Directory of glfw",
	default     = "vendor/glfw-3.3.2.bin.WIN64",
}

newoption {
	trigger     = "glfwdir32",
	value       = "PATH",
	description = "Directory of glfw",
	default     = "vendor/glfw-3.3.2.bin.WIN32",
}

newoption {
	trigger     = "with-asan",
	description = "Build with address sanitizer"
}

newoption {
	trigger     = "with-librw",
	description = "Build and use librw from this solution"
}

newoption {
	trigger     = "with-opus",
	description = "Build with opus"
}

newoption {
	trigger     = "lto",
	description = "Use link time optimization"
}

if(_OPTIONS["with-librw"]) then
	Librw = "vendor/librw"
else
	Librw = os.getenv("LIBRW") or "vendor/librw"
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
	elseif a == 'ARM64' then
		return 'arm64'
	end
	return a
end

workspace "re3"
	language "C++"
	configurations { "Debug", "Release", "Vanilla" }
	startproject "re3"
	location "build"
	symbols "Full"
	staticruntime "off"

	if _OPTIONS["with-asan"] then
		buildoptions { "-fsanitize=address -g3 -fno-omit-frame-pointer" }
		linkoptions { "-fsanitize=address" }
	end

	filter { "system:windows" }
		platforms {
			"win-x86-RW33_d3d8-mss",
			"win-x86-librw_d3d9-mss",
			"win-x86-librw_gl3_glfw-mss",
			"win-x86-RW33_d3d8-oal",
			"win-x86-librw_d3d9-oal",
			"win-x86-librw_gl3_glfw-oal",
			"win-amd64-librw_d3d9-oal",
			"win-amd64-librw_gl3_glfw-oal",
		}

	filter { "system:linux" }
		platforms {
			"linux-x86-librw_gl3_glfw-oal",
			"linux-amd64-librw_gl3_glfw-oal",
			"linux-arm-librw_gl3_glfw-oal",
			"linux-arm64-librw_gl3_glfw-oal",
		}

	filter { "system:bsd" }
		platforms {
			"bsd-x86-librw_gl3_glfw-oal",
			"bsd-amd64-librw_gl3_glfw-oal",
			"bsd-arm-librw_gl3_glfw-oal",
			"bsd-arm64-librw_gl3_glfw-oal"
		}

	filter { "system:macosx" }
		platforms {
			"macosx-arm64-librw_gl3_glfw-oal",
			"macosx-amd64-librw_gl3_glfw-oal",
		}

	filter "configurations:Debug"
		defines { "DEBUG" }
		
	filter "configurations:not Debug"
		defines { "NDEBUG" }
		optimize "Speed"
		if(_OPTIONS["lto"]) then
			flags { "LinkTimeOptimization" }
		end

	filter "configurations:Vanilla"
		defines { "VANILLA_DEFINES" }

	filter { "platforms:win*" }
		system "windows"

	filter { "platforms:linux*" }
		system "linux"
		
	filter { "platforms:bsd*" }
		system "bsd"

	filter { "platforms:macosx*" }
		system "macosx"
	
	filter { "platforms:*x86*" }
		architecture "x86"
		
	filter { "platforms:*amd64*" }
		architecture "amd64"

	filter { "platforms:*arm*" }
		architecture "ARM"

	filter { "platforms:macosx-arm64-*" }
		buildoptions { "-target", "arm64-apple-macos11", "-std=gnu++14" }

	filter { "platforms:macosx-amd64-*" }
		buildoptions { "-target", "x86_64-apple-macos10.12", "-std=gnu++14" }

	filter { "platforms:*librw_d3d9*" }
		defines { "RW_D3D9" }
		if(not _OPTIONS["with-librw"]) then
			libdirs { path.join(Librw, "lib/win-%{getarch(cfg.architecture)}-d3d9/%{cfg.buildcfg}") }
		end
		
	filter "platforms:*librw_gl3_glfw*"
		defines { "RW_GL3" }
		includedirs { path.join(_OPTIONS["glewdir"], "include") }
		if(not _OPTIONS["with-librw"]) then
			libdirs { path.join(Librw, "lib/%{getsys(cfg.system)}-%{getarch(cfg.architecture)}-gl3/%{cfg.buildcfg}") }
		end
		
	filter "platforms:*x86-librw_gl3_glfw*"
		includedirs { path.join(_OPTIONS["glfwdir32"], "include") }
		
	filter "platforms:*amd64-librw_gl3_glfw*"
		includedirs { path.join(_OPTIONS["glfwdir64"], "include") }

	filter "platforms:win*librw_gl3_glfw*"
		defines { "GLEW_STATIC" }

	filter  {}
		
    function setpaths (gamepath, exepath)
       if (gamepath) then
          postbuildcommands {
             '{COPYFILE} "%{cfg.buildtarget.abspath}" "' .. gamepath .. '%{cfg.buildtarget.name}"'
          }
          debugdir (gamepath)
          if (exepath) then
			 -- Used VS variable $(TargetFileName) because it doesn't accept premake tokens. Does debugcommand even work outside VS??
             debugcommand (gamepath .. "$(TargetFileName)")
             dir, file = exepath:match'(.*/)(.*)'
             debugdir (gamepath .. (dir or ""))
          end
       end
    end

if(_OPTIONS["with-librw"]) then
project "librw"
	kind "StaticLib"
	targetname "rw"
	targetdir(path.join(Librw, "lib/%{cfg.platform}/%{cfg.buildcfg}"))
	files { path.join(Librw, "src/*.*") }
	files { path.join(Librw, "src/*/*.*") }
	
	filter { "platforms:*x86*" }
		architecture "x86"

	filter { "platforms:*amd64*" }
		architecture "amd64"

	filter "platforms:win*"
		defines { "_CRT_SECURE_NO_WARNINGS", "_CRT_NONSTDC_NO_DEPRECATE" }
		staticruntime "on"
		buildoptions { "/Zc:sizedDealloc-" }

	filter "platforms:bsd*"
		includedirs { "/usr/local/include" }
		libdirs { "/usr/local/lib" }

	filter "platforms:macosx*"
		-- Support MacPorts and Homebrew
		includedirs { "/opt/local/include" }
		includedirs {"/usr/local/include" }
		libdirs { "/opt/local/lib" }
		libdirs { "/usr/local/lib" }

	filter "platforms:*gl3_glfw*"
		staticruntime "off"
	
	filter "platforms:*RW33*"
		flags { "ExcludeFromBuild" }
	filter  {}
end

local function addSrcFiles( prefix )
	return prefix .. "/*cpp", prefix .. "/*.h", prefix .. "/*.c", prefix .. "/*.ico", prefix .. "/*.rc"
end

project "re3"
	kind "WindowedApp"
	targetname "re3"
	targetdir "bin/%{cfg.platform}/%{cfg.buildcfg}"

	if(_OPTIONS["with-librw"]) then
		dependson "librw"
	end

	files { addSrcFiles("src") }
	files { addSrcFiles("src/animation") }
	files { addSrcFiles("src/audio") }
	files { addSrcFiles("src/audio/eax") }
	files { addSrcFiles("src/audio/oal") }
	files { addSrcFiles("src/buildings") }
	files { addSrcFiles("src/collision") }
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
	files { "src/extras/GitSHA1.cpp" } -- this won't be in repo in first build

	includedirs { "src" }
	includedirs { "src/animation" }
	includedirs { "src/audio" }
	includedirs { "src/audio/eax" }
	includedirs { "src/audio/oal" }
	includedirs { "src/buildings" }
	includedirs { "src/collision" }
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
	
	if _OPTIONS["with-opus"] then
		includedirs { "vendor/ogg/include" }
		includedirs { "vendor/opus/include" }
		includedirs { "vendor/opusfile/include" }
	end

	filter "platforms:*mss"
		defines { "AUDIO_MSS" }
		includedirs { "vendor/milessdk/include" }
		libdirs { "vendor/milessdk/lib" }
	
	if _OPTIONS["with-opus"] then
		filter "platforms:win*"
			libdirs { "vendor/ogg/win32/VS2015/Win32/%{cfg.buildcfg}" }
			libdirs { "vendor/opus/win32/VS2015/Win32/%{cfg.buildcfg}" }
			libdirs { "vendor/opusfile/win32/VS2015/Win32/Release-NoHTTP" }
		filter {}
		defines { "AUDIO_OPUS" }
	end
		
	filter "platforms:*oal"
		defines { "AUDIO_OAL" }
	
	filter {}
	if(os.getenv("GTA_III_RE_DIR")) then
		setpaths(os.getenv("GTA_III_RE_DIR") .. "/", "%(cfg.buildtarget.name)")
	end
	
	filter "platforms:win*"
		files { addSrcFiles("src/skel/win") }
		includedirs { "src/skel/win" }
		buildoptions { "/Zc:sizedDealloc-" }
		linkoptions "/SAFESEH:NO"
		characterset ("MBCS")
		targetextension ".exe"
		if(_OPTIONS["with-librw"]) then
			-- external librw is dynamic
			staticruntime "on"
		end
		prebuildcommands { '"%{prj.location}..\\printHash.bat" "%{prj.location}..\\src\\extras\\GitSHA1.cpp"' }
	
	filter "platforms:not win*"
		prebuildcommands { '"%{prj.location}/../printHash.sh" "%{prj.location}/../src/extras/GitSHA1.cpp"' }

	filter "platforms:win*glfw*"
		staticruntime "off"
		
	filter "platforms:win*oal"
		includedirs { "vendor/openal-soft/include" }
		includedirs { "vendor/libsndfile/include" }
		includedirs { "vendor/mpg123/include" }
		
	filter "platforms:win-x86*oal"
		libdirs { "vendor/mpg123/lib/Win32" }
		libdirs { "vendor/libsndfile/lib/Win32" }
		libdirs { "vendor/openal-soft/libs/Win32" }
		
	filter "platforms:win-amd64*oal"
		libdirs { "vendor/mpg123/lib/Win64" }
		libdirs { "vendor/libsndfile/lib/Win64" }
		libdirs { "vendor/openal-soft/libs/Win64" }

	filter "platforms:linux*oal"
		links { "openal", "mpg123", "sndfile", "pthread" }
		
	filter "platforms:bsd*oal"
		links { "openal", "mpg123", "sndfile", "pthread" }

	filter "platforms:macosx*oal"
		links { "openal", "mpg123", "sndfile", "pthread" }
		includedirs { "/usr/local/opt/openal-soft/include" }
		libdirs { "/usr/local/opt/openal-soft/lib" }
	
	if _OPTIONS["with-opus"] then
		filter {}
		links { "libogg" }
		links { "opus" }
		links { "opusfile" }
	end

	filter "platforms:*RW33*"
		includedirs { "sdk/rwsdk/include/d3d8" }
		libdirs { "sdk/rwsdk/lib/d3d8/release" }
		links { "rwcore", "rpworld", "rpmatfx", "rpskin", "rphanim", "rtbmp", "rtquat", "rtcharse", "rpanisot" }
		defines { "RWLIBS" }
		linkoptions "/SECTION:_rwcseg,ER!W /MERGE:_rwcseg=.text"
	
	filter "platforms:*librw*"
		defines { "LIBRW" }
		files { addSrcFiles("src/fakerw") }
		includedirs { "src/fakerw" }
		includedirs { Librw }
		if(_OPTIONS["with-librw"]) then
			libdirs { "vendor/librw/lib/%{cfg.platform}/%{cfg.buildcfg}" }
		end
		links { "rw" }

	filter "platforms:*d3d9*"
		defines { "USE_D3D9" }
		links { "d3d9" }
		
	filter "platforms:*x86*d3d*"
		includedirs { "sdk/dx8sdk/include" }
		libdirs { "sdk/dx8sdk/lib" }
		
	filter "platforms:win-x86*gl3_glfw*"
		libdirs { path.join(_OPTIONS["glewdir"], "lib/Release/Win32") }
		libdirs { path.join(_OPTIONS["glfwdir32"], "lib-" .. string.gsub(_ACTION or '', "vs", "vc")) }
		links { "opengl32", "glew32s", "glfw3" }
		
	filter "platforms:win-amd64*gl3_glfw*"
		libdirs { path.join(_OPTIONS["glewdir"], "lib/Release/x64") }
		libdirs { path.join(_OPTIONS["glfwdir64"], "lib-" .. string.gsub(_ACTION or '', "vs", "vc")) }
		links { "opengl32", "glew32s", "glfw3" }

	filter "platforms:linux*gl3_glfw*"
		links { "GL", "GLEW", "glfw" }
		
	filter "platforms:bsd*gl3_glfw*"
		links { "GL", "GLEW", "glfw", "sysinfo" }
		includedirs { "/usr/local/include" }
		libdirs { "/usr/local/lib" }

	filter "platforms:macosx*gl3_glfw*"
		links { "GLEW", "glfw" }
		linkoptions { "-framework OpenGL" }
		includedirs { "/opt/local/include" }
		includedirs { "/usr/local/include" }
		libdirs { "/opt/local/lib" }
		libdirs { "/usr/local/lib" }
