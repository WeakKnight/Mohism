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
        "mohism/**.cpp"
    }

    includedirs
    {
        "3rdparty/spdlog/include"
    }

    filter "system:macosx"
        cppdialect "C++17"
        staticruntime "On"
        systemversion "latest"

        defines
        {
            "MH_PLATFORM_MACOS",
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