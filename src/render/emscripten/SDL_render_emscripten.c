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

/*?*/
static int emscripten_canvas_create(int w, int h)
{
    return EM_ASM_INT({
        var canvas = document.createElement("canvas");
        canvas.width = $0;
        canvas.height = $1;

        SDL2.canvas.canvases.push(canvas);
        SDL2.canvas.contexts.push(null);
        return SDL2.canvas.canvases.length - 1;
    }, w, h);
}

static void emscripten_canvas_destroy(int id)
{
    EM_ASM_ARGS(
    {
        var canvas = SDL2.canvas.canvases[$0];
        if (canvas) {
            SDL2.canvas.canvases[$0] = null;
            SDL2.canvas.contexts[$0] = null;
        }
    }, id);
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
static void emscripten_canvas_2d_save(int id)
{
    EM_ASM_ARGS(
    {
        var ctx = SDL2.canvas.contexts[$0];
        if (ctx) {
            ctx.save();
        }
    }, id);
}

static void emscripten_canvas_2d_restore(int id)
{
    EM_ASM_ARGS(
    {
        var ctx = SDL2.canvas.contexts[$0];
        if (ctx) {
            ctx.restore();
        }
    }, id);
}

static void emscripten_canvas_2d_scale(int id, double x, double y)
{
    EM_ASM_ARGS(
    {
        var ctx = SDL2.canvas.contexts[$0];
        if (ctx) {
            ctx.scale($1, $2);
        }
    }, id, x, y);
}

static void emscripten_canvas_2d_rotate(int id, double angle)
{
    EM_ASM_ARGS(
    {
        var ctx = SDL2.canvas.contexts[$0];
        if (ctx) {
            ctx.rotate($1);
        }
    }, id, angle);
}

static void emscripten_canvas_2d_translate(int id, double x, double y)
{
    EM_ASM_ARGS(
    {
        var ctx = SDL2.canvas.contexts[$0];
        if (ctx) {
            ctx.translate($1, $2);
        }
    }, id, x, y);
}

static void emscripten_canvas_2d_set_image_smoothing(int id, int enabled)
{
    EM_ASM_ARGS(
    {
        var ctx = SDL2.canvas.contexts[$0];
        if (ctx) {
            if (typeof ctx.imageSmoothingEnabled !== 'undefined') {
                ctx.imageSmoothingEnabled = $1 != 0;
            } else if (typeof ctx.mozImageSmoothingEnabled !== 'undefined') {
                ctx.mozImageSmoothingEnabled = $1 != 0;
            } else if (typeof ctx.webkitImageSmoothingEnabled !== 'undefined') {
                ctx.webkitImageSmoothingEnabled = $1 != 0;
            }  else if (typeof ctx.msImageSmoothingEnabled !== 'undefined') {
                ctx.msImageSmoothingEnabled = $1 != 0;
            }
        }
    }, id, enabled);
}

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

static void emscripten_canvas_2d_clip(int id)
{
    EM_ASM_ARGS(
    {
        var ctx = SDL2.canvas.contexts[$0];
        if (ctx) {
            ctx.clip();
        }
    }, id);
}

/* other variants of this? names?*/
static void emscripten_canvas_2d_draw_canvas(int id, int srcId, double sx, double sy, double sw, double sh, double dx, double dy, double dw, double dh)
{
    EM_ASM_ARGS(
    {
        var ctx = SDL2.canvas.contexts[$0];
        var srcCanvas = SDL2.canvas.canvases[$1];
        if (ctx && srcCanvas) {
            ctx.drawImage(srcCanvas, $2, $3, $4, $5, $6, $7, $8, $9);
        }
    }, id, srcId, sx, sy, sw, sh, dx, dy, dw, dh);
}

static void emscripten_canvas_2d_put_image_data(int id, const void *data, double x, double y, int w, int h, int use_alpha)
{
    EM_ASM_ARGS(
    {
        var ctx = SDL2.canvas.contexts[$0];
        if (ctx) {
            //based on library_sdl.js SDL_UnlockSurface
            var image = ctx.createImageData($4, $5);
            var data = image.data;
            var src = $1 >> 2;
            var dst = 0;
            var num;
            if (typeof CanvasPixelArray !== 'undefined' && data instanceof CanvasPixelArray) {
                // IE10/IE11: ImageData objects are backed by the deprecated CanvasPixelArray,
                // not UInt8ClampedArray. These don't have buffers, so we need to revert
                // to copying a byte at a time. We do the undefined check because modern
                // browsers do not define CanvasPixelArray anymore.
                num = data.length;
                while (dst < num) {
                    var val = HEAP32[src]; // This is optimized. Instead, we could do {{{ makeGetValue('buffer', 'dst', 'i32') }}};
                    data[dst  ] = val & 0xff;
                    data[dst+1] = (val >> 8) & 0xff;
                    data[dst+2] = (val >> 16) & 0xff;
                    data[dst+3] = !$6 ? 0xff : ((val >> 24) & 0xff);
                    src++;
                    dst += 4;
                }
            } else {
                var data32 = new Uint32Array(data.buffer);
                num = data32.length;
                if (!$6) {
                    while (dst < num) {
                        // HEAP32[src++] is an optimization. Instead, we could do {{{ makeGetValue('buffer', 'dst', 'i32') }}};
                        data32[dst++] = HEAP32[src++] | 0xff000000;
                    }
                } else {
                    while (dst < num) {
                        data32[dst++] = HEAP32[src++];
                    }
                }
            }

            ctx.putImageData(image, $2, $3);
        }
    }, id, data, x, y, w, h, use_alpha);
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

static void emscripten_canvas_2d_rect(int id, double x, double y, double w, double h)
{
    EM_ASM_ARGS(
    {
        var ctx = SDL2.canvas.contexts[$0];
        if (ctx) {
            ctx.rect($1, $2, $3, $4);
        }
    }, id, x, y, w, h);
}


/* */

#define PI   3.14159265358979f

#define degToRad(x) ((x)*PI/180.f)

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
        {SDL_PIXELFORMAT_ABGR8888,
        SDL_PIXELFORMAT_BGR888},
        0,
        0
    }
};

/*************************************************************************************************
 * Context structures                                                                            *
 *************************************************************************************************/

typedef struct Emscripten_TextureData
{
    int canvas;
    void *pixel_data;
    size_t pitch;
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
static void Emscripten_UpdateViewClip(SDL_Renderer *renderer);

static void
Emscripten_WindowEvent(SDL_Renderer * renderer, const SDL_WindowEvent *event)
{

}

static int
Emscripten_UpdateViewport(SDL_Renderer * renderer)
{
    Emscripten_DriverData *data = (Emscripten_DriverData *)renderer->driverdata;

    emscripten_canvas_2d_restore(data->default_canvas);
    emscripten_canvas_2d_save(data->default_canvas);
    Emscripten_UpdateViewClip(renderer);
    return 0;
}

static int
Emscripten_UpdateClipRect(SDL_Renderer * renderer)
{
    Emscripten_DriverData *data = (Emscripten_DriverData *)renderer->driverdata;

    emscripten_canvas_2d_restore(data->default_canvas);
    emscripten_canvas_2d_save(data->default_canvas);
    Emscripten_UpdateViewClip(renderer);
    return 0;
}

static void
Emscripten_DestroyRenderer(SDL_Renderer *renderer)
{
    Emscripten_DriverData *data = (Emscripten_DriverData *)renderer->driverdata;

    SDL_free(data);
    SDL_free(renderer);
}

static void
Emscripten_UpdateViewClip(SDL_Renderer *renderer)
{
    Emscripten_DriverData *data = (Emscripten_DriverData *)renderer->driverdata;
    const SDL_Rect *vRect = &renderer->viewport;

    emscripten_canvas_2d_begin_path(data->default_canvas);

    if (renderer->clipping_enabled) {
        const SDL_Rect *cRect = &renderer->clip_rect;
        SDL_Rect r = *vRect;

        SDL_IntersectRect(cRect, vRect, &r);

        emscripten_canvas_2d_rect(data->default_canvas, r.x, r.y, r.w, r.h);

    } else {
        emscripten_canvas_2d_rect(data->default_canvas, vRect->x, vRect->y, vRect->w, vRect->h);
    }
    emscripten_canvas_2d_clip(data->default_canvas);
    emscripten_canvas_2d_translate(data->default_canvas, vRect->x, vRect->y);
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
    Emscripten_TextureData *data;

    /* Allocate a texture struct */
    data = (Emscripten_TextureData *)SDL_calloc(1, sizeof(Emscripten_TextureData));
    if (!data) {
        return SDL_OutOfMemory();
    }

    data->canvas = emscripten_canvas_create(texture->w, texture->h);
    if(emscripten_canvas_create_context(data->canvas, EMSCRIPTEN_CANVAS_CONTEXT_2D) < 0) {
        SDL_free(data);
        return -1;
    }

    /* Allocate a blob for image renderdata */
    if (texture->access == SDL_TEXTUREACCESS_STREAMING) {
        size_t size;
        data->pitch = texture->w * SDL_BYTESPERPIXEL(texture->format);
        size = texture->h * data->pitch;

        data->pixel_data = SDL_calloc(1, size);
        if (!data->pixel_data) {
            SDL_free(data);
            return SDL_OutOfMemory();
        }
    }

    texture->driverdata = data;

    return 0;
}

static int
Emscripten_UpdateTexture(SDL_Renderer *renderer, SDL_Texture *texture, const SDL_Rect *rect,
                    const void *pixels, int pitch)
{
    Emscripten_TextureData *tdata = (Emscripten_TextureData *)texture->driverdata;
    Uint8 *blob = NULL;
    Uint8 *src;
    int srcPitch;
    int y;

    /* Bail out if we're supposed to update an empty rectangle */
    if (rect->w <= 0 || rect->h <= 0)
        return 0;

    /* Reformat the texture data into a tightly packed array */
    srcPitch = rect->w * SDL_BYTESPERPIXEL(texture->format);
    src = (Uint8 *)pixels;
    if (pitch != srcPitch) {
        blob = (Uint8 *)SDL_malloc(srcPitch * rect->h);
        if (!blob) {
            return SDL_OutOfMemory();
        }
        src = blob;
        for (y = 0; y < rect->h; ++y)
        {
            SDL_memcpy(src, pixels, srcPitch);
            src += srcPitch;
            pixels = (Uint8 *)pixels + pitch;
        }
        src = blob;
    }

    emscripten_canvas_2d_put_image_data(tdata->canvas, src, rect->x, rect->y, rect->w, rect->h, texture->format == SDL_PIXELFORMAT_ABGR8888);
    SDL_free(blob);

    return 0;
}

static int
Emscripten_LockTexture(SDL_Renderer *renderer, SDL_Texture *texture, const SDL_Rect *rect,
                  void **pixels, int *pitch)
{
    Emscripten_TextureData *tdata = (Emscripten_TextureData *)texture->driverdata;

    /* Retrieve the buffer/pitch for the specified region */
    *pixels = (Uint8 *)tdata->pixel_data +
              (tdata->pitch * rect->y) +
              (rect->x * SDL_BYTESPERPIXEL(texture->format));
    *pitch = tdata->pitch;

    return 0;
}

static void
Emscripten_UnlockTexture(SDL_Renderer *renderer, SDL_Texture *texture)
{
    Emscripten_TextureData *tdata = (Emscripten_TextureData *)texture->driverdata;
    SDL_Rect rect;

    /* We do whole texture updates, at least for now */
    rect.x = 0;
    rect.y = 0;
    rect.w = texture->w;
    rect.h = texture->h;
    Emscripten_UpdateTexture(renderer, texture, &rect, tdata->pixel_data, tdata->pitch);
}

static int
Emscripten_SetRenderTarget(SDL_Renderer * renderer, SDL_Texture * texture)
{
    return 1;
}

static void
Emscripten_DestroyTexture(SDL_Renderer *renderer, SDL_Texture *texture)
{
    Emscripten_TextureData *tdata = (Emscripten_TextureData *)texture->driverdata;

    /* Destroy the texture */
    if (tdata) {
        emscripten_canvas_destroy(tdata->canvas);
        SDL_free(tdata->pixel_data);
        SDL_free(tdata);
        texture->driverdata = NULL;
    }
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

    /* undo clipping */
    emscripten_canvas_2d_restore(data->default_canvas);

    /* cache this? */
    emscripten_canvas_get_size(data->default_canvas, &w, &h);

    SDL_snprintf(buf, 30, "rgba(%i,%i,%i,%f)", renderer->r, renderer->g, renderer->b, renderer->a / 255.0f);
    emscripten_canvas_2d_set_fill_style(data->default_canvas, buf);
    emscripten_canvas_2d_fill_rect(data->default_canvas, 0, 0, w, h);

    emscripten_canvas_2d_save(data->default_canvas);
    Emscripten_UpdateViewClip(renderer);

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
    double x, y;

    SDL_snprintf(buf, 30, "rgba(%i,%i,%i,%f)", renderer->r, renderer->g, renderer->b, renderer->a / 255.0f);
    emscripten_canvas_2d_set_stroke_style(data->default_canvas, buf);

    emscripten_canvas_2d_begin_path(data->default_canvas);
    x = points[0].x;
    y = points[0].y;
    if (points[1].x != points[0].x) {
        y += 0.5;
    }
    if (points[1].y != points[0].y) {
        x += 0.5;
    }

    emscripten_canvas_2d_move_to(data->default_canvas, x, y);

    for (idx = 1; idx < count; ++idx) {
        x = points[idx].x;
        y = points[idx].y;
        if (idx < count - 1 || points[idx].x != points[idx - 1].x) {
            y += 0.5;
        }
        if (idx < count - 1 || points[idx].y != points[idx - 1].y) {
            x += 0.5;
        }
        emscripten_canvas_2d_line_to(data->default_canvas, x, y);
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
    Emscripten_DriverData *data = (Emscripten_DriverData *)renderer->driverdata;
    Emscripten_TextureData *tdata = (Emscripten_TextureData *)texture->driverdata;

    emscripten_canvas_2d_draw_canvas(data->default_canvas, tdata->canvas, srcrect->x, srcrect->y, srcrect->w, srcrect->h, dstrect->x, dstrect->y, dstrect->w, dstrect->h);
    return 0;
}

static int
Emscripten_RenderCopyEx(SDL_Renderer *renderer, SDL_Texture *texture, const SDL_Rect *srcrect,
                 const SDL_FRect *dstrect, const double angle, const SDL_FPoint *center, const SDL_RendererFlip flip)
{
    Emscripten_DriverData *data = (Emscripten_DriverData *)renderer->driverdata;
    Emscripten_TextureData *tdata = (Emscripten_TextureData *)texture->driverdata;

    emscripten_canvas_2d_save(data->default_canvas);

    emscripten_canvas_2d_translate(data->default_canvas, center->x + dstrect->x, center->y + dstrect->y);
    emscripten_canvas_2d_rotate(data->default_canvas, degToRad(angle));
    emscripten_canvas_2d_scale(data->default_canvas, flip & SDL_FLIP_HORIZONTAL ? -1 : 1, flip & SDL_FLIP_VERTICAL ? -1 : 1);
    emscripten_canvas_2d_translate(data->default_canvas, -center->x, -center->y);

    emscripten_canvas_2d_draw_canvas(data->default_canvas, tdata->canvas, srcrect->x, srcrect->y, srcrect->w, srcrect->h, 0, 0, dstrect->w, dstrect->h);

    emscripten_canvas_2d_restore(data->default_canvas);
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

    /* setup rendering quality */
    const char *hint = SDL_GetHint(SDL_HINT_RENDER_SCALE_QUALITY);

    if (!hint || *hint == '0' || SDL_strcasecmp(hint, "nearest") == 0) {
        emscripten_canvas_2d_set_image_smoothing(data->default_canvas, 0);
    } else {
        emscripten_canvas_2d_set_image_smoothing(data->default_canvas, 1);
    }

    emscripten_canvas_2d_save(data->default_canvas);

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
