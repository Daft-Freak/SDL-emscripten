/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2014 Sam Lantinga <slouken@libsdl.org>

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

#if SDL_VIDEO_RENDER_EMSCRIPTEN && !SDL_RENDER_DISABLED

#include "SDL_hints.h"

#include "../SDL_sysrender.h"
#include "../../video/SDL_blit.h"

/*************************************************************************************************
 * Bootstrap data                                                                                *
 *************************************************************************************************/

static SDL_Renderer *Emscripten_CreateRenderer(SDL_Window *window, Uint32 flags);

SDL_RenderDriver Emscripten_RenderDriver = {
    Emscripten_CreateRenderer,
    {
        "emscripten",
        (SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE),
        2,
        {SDL_PIXELFORMAT_ARGB8888,
        SDL_PIXELFORMAT_RGB888},
        0,
        0
    }
};

/*************************************************************************************************
 * Context structures                                                                            *
 *************************************************************************************************/

typedef struct Emscripten_TextureData
{

} Emscripten_TextureData;

typedef struct Emscripten_DriverData
{

} Emscripten_DriverData;

/*************************************************************************************************
 * Renderer state APIs                                                                           *
 *************************************************************************************************/

static void Emscripten_WindowEvent(SDL_Renderer * renderer,
                              const SDL_WindowEvent *event);
static int Emscripten_UpdateViewport(SDL_Renderer * renderer);
static void Emscripten_DestroyRenderer(SDL_Renderer *renderer);

static void
Emscripten_WindowEvent(SDL_Renderer * renderer, const SDL_WindowEvent *event)
{

}

static int
Emscripten_UpdateViewport(SDL_Renderer * renderer)
{
    return 0;
}

static int
Emscripten_UpdateClipRect(SDL_Renderer * renderer)
{
    return 0;
}

static void
Emscripten_DestroyRenderer(SDL_Renderer *renderer)
{
    SDL_free(renderer);
}

/*************************************************************************************************
 * Texture APIs                                                                                  *
 *************************************************************************************************/

static int Emscripten_CreateTexture(SDL_Renderer *renderer, SDL_Texture *texture);
static int Emscripten_UpdateTexture(SDL_Renderer *renderer, SDL_Texture *texture, const SDL_Rect *rect,
                               const void *pixels, int pitch);
static int Emscripten_LockTexture(SDL_Renderer *renderer, SDL_Texture *texture, const SDL_Rect *rect,
                             void **pixels, int *pitch);
static void Emscripten_UnlockTexture(SDL_Renderer *renderer, SDL_Texture *texture);
static int Emscripten_SetRenderTarget(SDL_Renderer * renderer, SDL_Texture * texture);
static void Emscripten_DestroyTexture(SDL_Renderer *renderer, SDL_Texture *texture);


static int
Emscripten_CreateTexture(SDL_Renderer *renderer, SDL_Texture *texture)
{
    return 0;
}

static int
Emscripten_UpdateTexture(SDL_Renderer *renderer, SDL_Texture *texture, const SDL_Rect *rect,
                    const void *pixels, int pitch)
{
    return 0;
}

static int
Emscripten_LockTexture(SDL_Renderer *renderer, SDL_Texture *texture, const SDL_Rect *rect,
                  void **pixels, int *pitch)
{
    return 0;
}

static void
Emscripten_UnlockTexture(SDL_Renderer *renderer, SDL_Texture *texture)
{
}

static int
Emscripten_SetRenderTarget(SDL_Renderer * renderer, SDL_Texture * texture)
{
    return 1;
}

static void
Emscripten_DestroyTexture(SDL_Renderer *renderer, SDL_Texture *texture)
{

}

/*************************************************************************************************
 * Rendering functions                                                                           *
 *************************************************************************************************/

static int Emscripten_RenderClear(SDL_Renderer *renderer);
static int Emscripten_RenderDrawPoints(SDL_Renderer *renderer, const SDL_FPoint *points, int count);
static int Emscripten_RenderDrawLines(SDL_Renderer *renderer, const SDL_FPoint *points, int count);
static int Emscripten_RenderFillRects(SDL_Renderer *renderer, const SDL_FRect *rects, int count);
static int Emscripten_RenderCopy(SDL_Renderer *renderer, SDL_Texture *texture, const SDL_Rect *srcrect,
                            const SDL_FRect *dstrect);
static int Emscripten_RenderCopyEx(SDL_Renderer * renderer, SDL_Texture * texture,
                         const SDL_Rect * srcrect, const SDL_FRect * dstrect,
                         const double angle, const SDL_FPoint *center, const SDL_RendererFlip flip);
static int Emscripten_RenderReadPixels(SDL_Renderer * renderer, const SDL_Rect * rect,
                    Uint32 pixel_format, void * pixels, int pitch);
static void Emscripten_RenderPresent(SDL_Renderer *renderer);

static int
Emscripten_RenderClear(SDL_Renderer * renderer)
{
    return 0;
}

static int
Emscripten_RenderDrawPoints(SDL_Renderer *renderer, const SDL_FPoint *points, int count)
{
    return 0;
}

static int
Emscripten_RenderDrawLines(SDL_Renderer *renderer, const SDL_FPoint *points, int count)
{
    return 0;
}

static int
Emscripten_RenderFillRects(SDL_Renderer *renderer, const SDL_FRect *rects, int count)
{
    return 0;
}

static int
Emscripten_RenderCopy(SDL_Renderer *renderer, SDL_Texture *texture, const SDL_Rect *srcrect,
                 const SDL_FRect *dstrect)
{
    return 0;
}

static int
Emscripten_RenderCopyEx(SDL_Renderer *renderer, SDL_Texture *texture, const SDL_Rect *srcrect,
                 const SDL_FRect *dstrect, const double angle, const SDL_FPoint *center, const SDL_RendererFlip flip)
{
    return 0;
}

static int
Emscripten_RenderReadPixels(SDL_Renderer * renderer, const SDL_Rect * rect,
                    Uint32 pixel_format, void * pixels, int pitch)
{
    return 0;
}

static void
Emscripten_RenderPresent(SDL_Renderer *renderer)
{

}


/*************************************************************************************************
 * Renderer instantiation                                                                        *
 *************************************************************************************************/

static SDL_Renderer *
Emscripten_CreateRenderer(SDL_Window *window, Uint32 flags)
{
    SDL_Renderer *renderer;
    Emscripten_DriverData *data;

    /* Create the renderer struct */
    renderer = (SDL_Renderer *)SDL_calloc(1, sizeof(SDL_Renderer));
    if (!renderer) {
        SDL_OutOfMemory();
        return NULL;
    }

    data = (Emscripten_DriverData *)SDL_calloc(1, sizeof(Emscripten_DriverData));
    if (!data) {
        Emscripten_DestroyRenderer(renderer);
        SDL_OutOfMemory();
        return NULL;
    }
    renderer->info = Emscripten_RenderDriver.info;
    renderer->info.flags = (SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
    renderer->driverdata = data;
    renderer->window = window;


    /* Populate the function pointers for the module */
    renderer->WindowEvent         = &Emscripten_WindowEvent;
    renderer->CreateTexture       = &Emscripten_CreateTexture;
    renderer->UpdateTexture       = &Emscripten_UpdateTexture;
    renderer->LockTexture         = &Emscripten_LockTexture;
    renderer->UnlockTexture       = &Emscripten_UnlockTexture;
    renderer->SetRenderTarget     = &Emscripten_SetRenderTarget;
    renderer->UpdateViewport      = &Emscripten_UpdateViewport;
    renderer->UpdateClipRect      = &Emscripten_UpdateClipRect;
    renderer->RenderClear         = &Emscripten_RenderClear;
    renderer->RenderDrawPoints    = &Emscripten_RenderDrawPoints;
    renderer->RenderDrawLines     = &Emscripten_RenderDrawLines;
    renderer->RenderFillRects     = &Emscripten_RenderFillRects;
    renderer->RenderCopy          = &Emscripten_RenderCopy;
    renderer->RenderCopyEx        = &Emscripten_RenderCopyEx;
    renderer->RenderReadPixels    = &Emscripten_RenderReadPixels;
    renderer->RenderPresent       = &Emscripten_RenderPresent;
    renderer->DestroyTexture      = &Emscripten_DestroyTexture;
    renderer->DestroyRenderer     = &Emscripten_DestroyRenderer;

    return renderer;
}

#endif /* SDL_VIDEO_RENDER_EMSCRIPTEN && !SDL_RENDER_DISABLED */

/* vi: set ts=4 sw=4 expandtab: */
