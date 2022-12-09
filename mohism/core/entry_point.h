#pragma once

#include "log.h"
#include "application.h"

#ifdef MH_PLATFORM_WINDOWS
int main(int argc, char const *argv[])
{
    /* code */
    return 0;
}
#endif

#ifdef MH_PLATFORM_MACOS

extern MH::Application* MH::create_application();

int main(int argc, char const *argv[])
{
    MH::Log::init();
    MH::Log::get_core_logger()->warn("Logger Initialized");
    
    MH::Application* app = MH::create_application();
    app->run();
    return 0;
}
#endif

