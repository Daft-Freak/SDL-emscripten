/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2020 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/
#include "../../SDL_internal.h"

#if SDL_VIDEO_DRIVER_32BLIT

#include "../SDL_sysvideo.h"
#include "SDL_32blitframebuffer_c.h"

#include "api.hpp"

int SDL_TTBlit_CreateWindowFramebuffer(_THIS, SDL_Window * window, Uint32 * format, void ** pixels, int *pitch)
{
    auto screen = blit::api.set_screen_mode(blit::ScreenMode::hires);
    *format = SDL_PIXELFORMAT_RGB24;
    *pixels = screen.data;
    *pitch = 320 * 3;

    return 0;
}

int SDL_TTBlit_UpdateWindowFramebuffer(_THIS, SDL_Window * window, const SDL_Rect * rects, int numrects)
{
    // nothing
    return 0;
}

void SDL_TTBlit_DestroyWindowFramebuffer(_THIS, SDL_Window * window)
{
    // nothing
}

#endif /* SDL_VIDEO_DRIVER_TTBlit */

/* vi: set ts=4 sw=4 expandtab: */
