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
#include "SDL_stdinc.h"
#include "SDL_rwops.h"

extern "C" {
#include "SDL_32blitrwops.h"
}

#include "../../core/32blit/api.hpp"

int SDLCALL
ttblit_open(SDL_RWops * context, const char *filename, const char *mode)
{
    int blit_mode = 0;
    if(SDL_strcmp(mode, "r") == 0 || SDL_strcmp(mode, "rb") == 0)
        blit_mode = blit::OpenMode::read;
    else if(SDL_strcmp(mode, "w") == 0 || SDL_strcmp(mode, "wb") == 0)
        blit_mode = blit::OpenMode::write;
    else if(SDL_strcmp(mode, "r+") == 0 || SDL_strcmp(mode, "rb+") == 0)
        blit_mode = blit::OpenMode::read | blit::OpenMode::write;
    else
        return SDL_SetError("ttblit_open: unhandled mode");

    // TODO: maybe define a type
    context->hidden.unknown.data1 = blit::api.open_file(filename, blit_mode);
    context->hidden.unknown.data2 = 0; // offset

    if(!context->hidden.unknown.data1)
        return SDL_SetError("ttblit_open: failed to open file");

    return 0;
}

Sint64 SDLCALL
ttblit_size(SDL_RWops * context)
{
    return blit::api.get_file_length(context->hidden.unknown.data1);
}

Sint64 SDLCALL
ttblit_seek(SDL_RWops * context, Sint64 offset, int whence)
{
    if(whence == RW_SEEK_SET)
        context->hidden.unknown.data2 = (void *)(intptr_t)offset;
    else if(whence == RW_SEEK_CUR)
        context->hidden.unknown.data2 = (void *)((intptr_t)context->hidden.unknown.data2 + offset);
    else
        context->hidden.unknown.data2 = (void *)(intptr_t)(ttblit_size(context) - offset);

    return (intptr_t)context->hidden.unknown.data2;
}

size_t SDLCALL
ttblit_read(SDL_RWops * context, void *ptr, size_t size, size_t maxnum)
{
    uint32_t offset = (intptr_t)context->hidden.unknown.data2;
    int32_t ret = blit::api.read_file(context->hidden.unknown.data1, offset, size * maxnum, (char *)ptr);

    context->hidden.unknown.data2 = (void *)(intptr_t)(offset + ret);

    return ret < 0 ? 0 : ret / size;
}

size_t SDLCALL
ttblit_write(SDL_RWops * context, const void *ptr, size_t size, size_t num)
{
    uint32_t offset = (intptr_t)context->hidden.unknown.data1;
    int32_t ret = blit::api.write_file(context->hidden.unknown.data1, offset, size * num, (const char *)ptr);

    context->hidden.unknown.data2 = (void *)(intptr_t)(offset + ret);

    return ret < 0 ? 0 : ret / size;
}

int SDLCALL
ttblit_close(SDL_RWops * context)
{
    blit::api.close_file(context->hidden.unknown.data1);
    SDL_FreeRW(context);
    return 0;
}
