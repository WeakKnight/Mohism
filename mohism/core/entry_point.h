#pragma once

#ifdef MH_PLATFORM_WINDOWS
int main(int argc, char const *argv[])
{
    /* code */
    return 0;
}
#endif

#ifdef MH_PLATFORM_MACOS

extern void game_initialize();

//void engine_entry();

int main(int argc, char const *argv[])
{
    game_initialize();
    return 0;
}
#endif

