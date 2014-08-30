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

#include <emscripten/emscripten.h>

/* WIP canvas API */
#define EMSCRIPTEN_CANVAS_CONTEXT_UNKNOWN -1
#define EMSCRIPTEN_CANVAS_CONTEXT_NONE     0
#define EMSCRIPTEN_CANVAS_CONTEXT_2D       1
#define EMSCRIPTEN_CANVAS_CONTEXT_WEBGL    2

static int emscripten_canvas_get_default()
{
    return EM_ASM_INT_V({
        var idx = SDL2.canvas.canvases.indexOf(Module['canvas']);

        if(idx != -1)
            return idx;

        SDL2.canvas.canvases.push(Module['canvas']);
        SDL2.canvas.contexts.push(Module['ctx']);
        return SDL2.canvas.canvases.length - 1;
    });
}

static void emscripten_canvas_get_size(int id, int *w, int *h)
{
    EM_ASM_ARGS(
    {
        var canvas = SDL2.canvas.canvases[$0];
        if (canvas) {
            HEAP32[$1 >> 2] = canvas.width;
            HEAP32[$2 >> 2] = canvas.height;
        } else {
            HEAP32[$1 >> 2] = 0;
            HEAP32[$2 >> 2] = 0;
        }
    }, id, w, h);
}

static int emscripten_canvas_get_context_type(int id)
{
    return EM_ASM_INT({
        var ctx = SDL2.canvas.contexts[$0];
        if (!ctx) {
            return 0; //NONE
        }

        if (ctx instanceof CanvasRenderingContext2D) {
            return 1; //2D
        } else if (ctx instanceof WebGLRenderingContext) {
            return 2; //WebGL
        }

        return -1; //INVALID
    }, id);
}

static int emscripten_canvas_create_context(int id, int type)
{
    return EM_ASM_INT({
        var canvas = SDL2.canvas.canvases[$0];
        if (!canvas) {
            return -1;
        }

        switch ($1) {
            case 1:
                SDL2.canvas.contexts[$0] = canvas.getContext("2d");
                break;
            case 2:
                SDL2.canvas.contexts[$0] = canvas.getContext("webgl");
                break;
            default:
                return -1;
        }

        if (SDL2.canvas.contexts[$0]) {
            return 0;
        }

        return -1;
    }, id, type);
}

/* 2d context */
static void emscripten_canvas_2d_set_stroke_style(int id, const char *style)
{
    EM_ASM_ARGS(
    {
        var ctx = SDL2.canvas.contexts[$0];
        if (ctx) {
            ctx.strokeStyle = Pointer_stringify($1);
        }
    }, id, style);
}

static void emscripten_canvas_2d_set_fill_style(int id, const char *style)
{
    EM_ASM_ARGS(
    {
        var ctx = SDL2.canvas.contexts[$0];
        if (ctx) {
            ctx.fillStyle = Pointer_stringify($1);
        }
    }, id, style);
}

static void emscripten_canvas_2d_fill_rect(int id, double x, double y, double w, double h)
{
    EM_ASM_ARGS(
    {
        var ctx = SDL2.canvas.contexts[$0];
        if (ctx) {
            ctx.fillRect($1, $2, $3, $4);
        }
    }, id, x, y, w, h);
}

static void emscripten_canvas_2d_begin_path(int id)
{
    EM_ASM_ARGS(
    {
        var ctx = SDL2.canvas.contexts[$0];
        if (ctx) {
            ctx.beginPath();
        }
    }, id);
}

static void emscripten_canvas_2d_fill(int id)
{
    EM_ASM_ARGS(
    {
        var ctx = SDL2.canvas.contexts[$0];
        if (ctx) {
            ctx.fill();
        }
    }, id);
}
static void emscripten_canvas_2d_stroke(int id)
{
    EM_ASM_ARGS(
    {
        var ctx = SDL2.canvas.contexts[$0];
        if (ctx) {
            ctx.stroke();
        }
    }, id);
}

static void emscripten_canvas_2d_move_to(int id, double x, double y)
{
    EM_ASM_ARGS(
    {
        var ctx = SDL2.canvas.contexts[$0];
        if (ctx) {
            ctx.moveTo($1, $2);
        }
    }, id, x, y);
}

static void emscripten_canvas_2d_line_to(int id, double x, double y)
{
    EM_ASM_ARGS(
    {
        var ctx = SDL2.canvas.contexts[$0];
        if (ctx) {
            ctx.lineTo($1, $2);
        }
    }, id, x, y);
}

/* */

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
    int default_canvas;
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
    Emscripten_DriverData *data = (Emscripten_DriverData *)renderer->driverdata;
    char buf[30];
    int w, h;

    /* cache this? */
    emscripten_canvas_get_size(data->default_canvas, &w, &h);

    SDL_snprintf(buf, 30, "rgba(%i,%i,%i,%f)", renderer->r, renderer->g, renderer->b, renderer->a / 255.0f);
    emscripten_canvas_2d_set_fill_style(data->default_canvas, buf);
    emscripten_canvas_2d_fill_rect(data->default_canvas, 0, 0, w, h);

    return 0;
}

static int
Emscripten_RenderDrawPoints(SDL_Renderer *renderer, const SDL_FPoint *points, int count)
{
    Emscripten_DriverData *data = (Emscripten_DriverData *)renderer->driverdata;
    char buf[30];
    int idx;

    SDL_snprintf(buf, 30, "rgba(%i,%i,%i,%f)", renderer->r, renderer->g, renderer->b, renderer->a / 255.0f);
    emscripten_canvas_2d_set_fill_style(data->default_canvas, buf);

    for (idx = 0; idx < count; ++idx) {
        const SDL_FPoint *point = &points[idx];
        emscripten_canvas_2d_fill_rect(data->default_canvas, point->x, point->y, 1, 1);
    }

    return 0;
}

static int
Emscripten_RenderDrawLines(SDL_Renderer *renderer, const SDL_FPoint *points, int count)
{
    Emscripten_DriverData *data = (Emscripten_DriverData *)renderer->driverdata;
    char buf[30];
    int idx;

    SDL_snprintf(buf, 30, "rgba(%i,%i,%i,%f)", renderer->r, renderer->g, renderer->b, renderer->a / 255.0f);
    emscripten_canvas_2d_set_stroke_style(data->default_canvas, buf);

    emscripten_canvas_2d_begin_path(data->default_canvas);
    emscripten_canvas_2d_move_to(data->default_canvas, points[0].x, points[0].y);

    for (idx = 1; idx < count; ++idx) {
        const SDL_FPoint *point = &points[idx];
        emscripten_canvas_2d_line_to(data->default_canvas, point->x, point->y);
    }
    emscripten_canvas_2d_stroke(data->default_canvas);
    return 0;
}

static int
Emscripten_RenderFillRects(SDL_Renderer *renderer, const SDL_FRect *rects, int count)
{
    Emscripten_DriverData *data = (Emscripten_DriverData *)renderer->driverdata;
    char buf[30];
    int idx;

    SDL_snprintf(buf, 30, "rgba(%i,%i,%i,%f)", renderer->r, renderer->g, renderer->b, renderer->a / 255.0f);
    emscripten_canvas_2d_set_fill_style(data->default_canvas, buf);

    for (idx = 0; idx < count; ++idx) {
        const SDL_FRect *rect = &rects[idx];
        emscripten_canvas_2d_fill_rect(data->default_canvas, rect->x, rect->y, rect->w, rect->h);
    }

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

    /* temp */
    EM_ASM({
        if(typeof(SDL2) === 'undefined')
            SDL2 = {};
        if(typeof(SDL2.canvas) === 'undefined')
            SDL2.canvas = {canvases: [], contexts: []};
    });

    /* init canvas */
    data->default_canvas = emscripten_canvas_get_default();

    if (emscripten_canvas_get_context_type(data->default_canvas) == EMSCRIPTEN_CANVAS_CONTEXT_NONE) {
        if (emscripten_canvas_create_context(data->default_canvas, EMSCRIPTEN_CANVAS_CONTEXT_2D) < 0) {
            Emscripten_DestroyRenderer(renderer);
            return NULL;
        }
    }

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
