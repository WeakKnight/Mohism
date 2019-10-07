#pragma once

#ifdef MH_PLATFORM_WINDOWS
    #ifdef MH_BUILD_DLL
        #define MH_API __declspec(dllexport)
    #else
        #define MH_API 
    #endif
#else
    #define MH_API
#endif 