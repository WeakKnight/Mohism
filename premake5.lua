workspace "mohism"
    location "build"
    architecture "arm64"
    startproject "sandbox"

    configurations
    {
        "debug",
        "release",
        "distribute"
    }

outputdir = "%{cfg.system}-%{cfg.buildcfg}"

include "3rdparty/glfw"

project "mohism"
    location "build"
    kind "SharedLib"
    language "C++"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("intermediate/" .. outputdir .. "/%{prj.name}")

    pchheader "../mohism/pch.h"
    pchsource "../mohism/pch.cpp"

    files
    {
        "mohism/**.h",
        "mohism/**.cpp",
        "mohism/**.c",
        "mohism/**.m",
    }

    includedirs
    {
        "3rdparty/glfw/include",
        "mohism/"
    }

    filter "system:macosx"
        cppdialect "C++17"
        staticruntime "On"
        systemversion "latest"

        defines
        {
            "MH_PLATFORM_MACOS",
            "GLFW_INCLUDE_NONE"
        }

        links 
        {
            --"libiconv",
            "glfw",
            "ForceFeedback.framework",
            "AudioToolbox.framework",
            "CoreAudio.framework",
            "CoreVideo.framework",
            "IOKit.framework",
            "Carbon.framework",
            "Cocoa.framework",
            "CoreFoundation.framework",
            "QuartzCore.framework",
            "Metal.framework"
        }

        buildoptions 
        {
            "-F /Library/Frameworks", 
            "-F ~/Library/Frameworks"
        }
    
    filter "configurations:debug"
        defines "MH_DEBUG"
        symbols "On"
    
	filter "configurations:release"
        defines "MH_RELEASE"
        optimize "On"

    filter "configurations:distribute"
        defines "MH_DISTRIBUTE"
        optimize "On"

project "sandbox"
    location "build"
    kind "ConsoleApp"
    language "c++"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("intermediate/" .. outputdir .. "/%{prj.name}")

    files
    {
        "sandbox/**.h",
        "sandbox/**.cpp",
        "sandbox/**.c"
    }

    includedirs
    {
        "mohism/"
    }

    links
    {
        "mohism"
    }

    filter "system:macosx"
        cppdialect "C++17"
        staticruntime "On"
        systemversion "latest"

        defines "MH_PLATFORM_MACOS"
    
    filter "configurations:debug"
        defines "MH_DEBUG"
        symbols "On"
    
    filter "configurations:release"
        defines "MH_RELEASE"
        optimize "On"

    filter "configurations:distribute"
        defines "MH_DISTRIBUTE"
        optimize "On"