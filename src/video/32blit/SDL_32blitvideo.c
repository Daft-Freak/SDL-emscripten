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

/* TTBlit SDL video driver implementation; this is just enough to make an
 *  SDL-based application THINK it's got a working video driver, for
 *  applications that call SDL_Init(SDL_INIT_VIDEO) when they don't need it,
 *  and also for use as a collection of stubs when porting SDL to a new
 *  platform for which you haven't yet written a valid video driver.
 *
 * This is also a great way to determine bottlenecks: if you think that SDL
 *  is a performance problem for a given platform, enable this driver, and
 *  then see if your application runs faster without video overhead.
 *
 * Initial work by Ryan C. Gordon (icculus@icculus.org). A good portion
 *  of this was cut-and-pasted from Stephane Peter's work in the AAlib
 *  SDL video driver.  Renamed to "TTBlit" by Sam Lantinga.
 */

#include "SDL_video.h"
#include "SDL_mouse.h"
#include "../SDL_sysvideo.h"
#include "../SDL_pixels_c.h"
#include "../../events/SDL_events_c.h"

#include "SDL_32blitvideo.h"
#include "SDL_32blitevents_c.h"
#include "SDL_32blitframebuffer_c.h"

#define TTBLIT_DRIVER_NAME "32Blit"

/* Initialization/Query functions */
static int TTBlit_VideoInit(_THIS);
static int TTBlit_SetDisplayMode(_THIS, SDL_VideoDisplay * display, SDL_DisplayMode * mode);
static void TTBlit_VideoQuit(_THIS);

static int TTBlit_CreateWindow(_THIS, SDL_Window * window);

/* 32Blit driver bootstrap functions */

static int
TTBlit_Available(void)
{
    return (1);
}

static void
TTBlit_DeleteDevice(SDL_VideoDevice * device)
{
    SDL_free(device);
}

static SDL_VideoDevice *
TTBlit_CreateDevice(int devindex)
{
    SDL_VideoDevice *device;

    /* Initialize all variables that we clean on shutdown */
    device = (SDL_VideoDevice *) SDL_calloc(1, sizeof(SDL_VideoDevice));
    if (!device) {
        SDL_OutOfMemory();
        return (0);
    }

    /* Set the function pointers */
    device->VideoInit = TTBlit_VideoInit;
    device->VideoQuit = TTBlit_VideoQuit;
    device->SetDisplayMode = TTBlit_SetDisplayMode;

    device->PumpEvents = TTBlit_PumpEvents;

    device->CreateSDLWindow = TTBlit_CreateWindow;

    device->CreateWindowFramebuffer = SDL_TTBlit_CreateWindowFramebuffer;
    device->UpdateWindowFramebuffer = SDL_TTBlit_UpdateWindowFramebuffer;
    device->DestroyWindowFramebuffer = SDL_TTBlit_DestroyWindowFramebuffer;

    device->free = TTBlit_DeleteDevice;

    return device;
}

VideoBootStrap TTBlit_bootstrap = {
    TTBLIT_DRIVER_NAME, "SDL 32Blit video driver",
    TTBlit_Available, TTBlit_CreateDevice
};


int
TTBlit_VideoInit(_THIS)
{
    SDL_DisplayMode mode;

    /* Use a fake 32-bpp desktop mode */
    mode.format = SDL_PIXELFORMAT_RGB888;
    mode.w = 320;
    mode.h = 240;
    mode.refresh_rate = 60;
    mode.driverdata = NULL;
    if (SDL_AddBasicVideoDisplay(&mode) < 0) {
        return -1;
    }

    SDL_zero(mode);
    SDL_AddDisplayMode(&_this->displays[0], &mode);

    /* We're done! */
    return 0;
}

static int
TTBlit_SetDisplayMode(_THIS, SDL_VideoDisplay * display, SDL_DisplayMode * mode)
{
    return 0;
}

void
TTBlit_VideoQuit(_THIS)
{
}


static int
TTBlit_CreateWindow(_THIS, SDL_Window * window)
{
    // only fullscreen supported, at one size
    window->flags |= SDL_WINDOW_FULLSCREEN;
    window->w = 320;
    window->h = 240;
    window->windowed.w = window->w;
    window->windowed.h = window->h;

    /* One window, it always has focus */
    SDL_SetMouseFocus(window);
    SDL_SetKeyboardFocus(window);

    return 0;
}


#endif /* SDL_VIDEO_DRIVER_32BLIT */

/* vi: set ts=4 sw=4 expandtab: */
