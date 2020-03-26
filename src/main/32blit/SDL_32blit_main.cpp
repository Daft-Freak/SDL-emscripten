#include <cstdint>
#include "SDL_config.h"
#include "SDL_main.h"

// called from user startup to do final init, usually implemented by startup_user.cpp
extern "C" void cpp_do_init()
{
    char *argv[2];
    argv[0] = SDL_strdup("app");
    argv[1] = 0;

    SDL_SetMainReady();

    if(main(1, argv) != 0)
    {
        while(true){} // FIXME: do something? we can't continue the app...
    }
}

extern void update(uint32_t);

// firmware expects this to be exported
// TODO?: this doesn't do a fixed step like BlitEngine does
namespace blit
{
    bool tick(uint32_t time)
    {
        update(time);
        return true;
    }
}