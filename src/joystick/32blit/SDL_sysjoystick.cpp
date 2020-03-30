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

#if defined(SDL_JOYSTICK_32BLIT)

#include "SDL_joystick.h"
#include "../SDL_sysjoystick.h"
#include "../SDL_joystick_c.h"

#include "../../core/32blit/api.hpp"

static int
TTBLIT_JoystickInit(void)
{
    return 0;
}

static int
TTBLIT_JoystickGetCount(void)
{
    return 1;
}

static void
TTBLIT_JoystickDetect(void)
{
}

static const char *
TTBLIT_JoystickGetDeviceName(int device_index)
{
    return "32Blit";
}

static int
TTBLIT_JoystickGetDevicePlayerIndex(int device_index)
{
    return -1;
}

static void
TTBLIT_JoystickSetDevicePlayerIndex(int device_index, int player_index)
{
}

static SDL_JoystickGUID
TTBLIT_JoystickGetDeviceGUID(int device_index)
{
    SDL_JoystickGUID guid;
    SDL_zero(guid);
    return guid;
}

static SDL_JoystickID
TTBLIT_JoystickGetDeviceInstanceID(int device_index)
{
    return 0;
}

static int
TTBLIT_JoystickOpen(SDL_Joystick * joystick, int device_index)
{
    if(device_index != 0)
        return SDL_SetError("Only one joystick device");

    joystick->instance_id = 0;
    joystick->naxes = 2;
    joystick->nhats = 0;
    joystick->nballs = 0;
    joystick->nbuttons = 11;

    return 0;
}

static int
TTBLIT_JoystickRumble(SDL_Joystick * joystick, Uint16 low_frequency_rumble, Uint16 high_frequency_rumble)
{
    return SDL_Unsupported();
}

static void
TTBLIT_JoystickUpdate(SDL_Joystick * joystick)
{
    for(int i = 0; i < joystick->nbuttons; i++)
        SDL_PrivateJoystickButton(joystick, i, (blit::api.buttons & (1 << i)) ? 1 : 0);

    SDL_PrivateJoystickAxis(joystick, 0, (Sint16)(blit::api.joystick.x * 32767.0f));
    SDL_PrivateJoystickAxis(joystick, 1, (Sint16)(blit::api.joystick.y * 32767.0f));
}

static void
TTBLIT_JoystickClose(SDL_Joystick * joystick)
{
}

static void
TTBLIT_JoystickQuit(void)
{
}

SDL_JoystickDriver SDL_32BLIT_JoystickDriver =
{
    TTBLIT_JoystickInit,
    TTBLIT_JoystickGetCount,
    TTBLIT_JoystickDetect,
    TTBLIT_JoystickGetDeviceName,
    TTBLIT_JoystickGetDevicePlayerIndex,
    TTBLIT_JoystickSetDevicePlayerIndex,
    TTBLIT_JoystickGetDeviceGUID,
    TTBLIT_JoystickGetDeviceInstanceID,
    TTBLIT_JoystickOpen,
    TTBLIT_JoystickRumble,
    TTBLIT_JoystickUpdate,
    TTBLIT_JoystickClose,
    TTBLIT_JoystickQuit,
};

#endif /* SDL_JOYSTICK_32BLIT || SDL_JOYSTICK_DISABLED */

/* vi: set ts=4 sw=4 expandtab: */
