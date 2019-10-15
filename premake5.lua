workspace "mohism"
    location "build"
    architecture "x64"
    startproject "sandbox"

    configurations
    {
        "debug",
        "release",
        "distribute"
    }

outputdir = "%{cfg.system}-%{cfg.buildcfg}"

include "3rdparty/glfw"
include "3rdparty/glad"
include "3rdparty/imgui"

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
        "3rdparty/glm/glm/**.hpp",
        "3rdparty/glm/glm/**.inl",
    }

    includedirs
    {
        "3rdparty/spdlog/include",
        "3rdparty/glfw/include",
        "3rdparty/glad/include",
        "3rdparty/imgui",
        "3rdparty/glm",
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
            "imgui",
            "glfw",
            "glad",
            "ForceFeedback.framework",
            "AudioToolbox.framework",
            "CoreAudio.framework",
            "CoreVideo.framework",
            "IOKit.framework",
            "Carbon.framework",
            "Cocoa.framework",
            "OpenGL.framework",
            "CoreFoundation.framework",
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
        "sandbox/**.cpp"
    }

    includedirs
    {
        "3rdparty/spdlog/include",
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