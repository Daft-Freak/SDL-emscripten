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

#ifndef SDL_32blitrwops_h
#define SDL_32blitrwops_h

int SDLCALL ttblit_open(SDL_RWops * context, const char *filename, const char *mode);
Sint64 SDLCALL ttblit_size(SDL_RWops * context);
Sint64 SDLCALL ttblit_seek(SDL_RWops * context, Sint64 offset, int whence);
size_t SDLCALL ttblit_read(SDL_RWops * context, void *ptr, size_t size, size_t maxnum);
size_t SDLCALL ttblit_write(SDL_RWops * context, const void *ptr, size_t size, size_t num);
int SDLCALL ttblit_close(SDL_RWops * context);

#endif
