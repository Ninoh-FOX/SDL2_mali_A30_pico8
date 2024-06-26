/*
  Customized version for Miyoo-Mini handheld.
  Only tested under Miyoo-Mini stock OS (original firmware) with Parasyte compatible layer.

  Copyright (C) 1997-2022 Sam Lantinga <slouken@libsdl.org>
  Copyright (C) 2022-2022 Steward Fu <steward.fu@gmail.com>

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

#if SDL_VIDEO_RENDER_MMIYOO

#include <unistd.h>
#include <stdbool.h>

#include "SDL_hints.h"
#include "../SDL_sysrender.h"
#include "../../video/mmiyoo/SDL_video_mmiyoo.h"
#include "../../video/mmiyoo/SDL_event_mmiyoo.h"

typedef struct MMIYOO_TextureData {
    void *data;
    unsigned int size;
    unsigned int width;
    unsigned int height;
    unsigned int bits;
    unsigned int format;
    unsigned int pitch;
} MMIYOO_TextureData;

typedef struct {
    SDL_Texture *boundTarget;
    SDL_bool initialized;
    unsigned int bpp;
    SDL_bool vsync;
} MMIYOO_RenderData;

#define MAX_TEXTURE 10

struct _NDS_TEXTURE {
    int pitch;
    const void *pixels;
    SDL_Texture *texture;
};

extern GFX gfx;
extern MMIYOO_EventInfo evt;
extern SDL_Surface *fps_info;
extern int FB_W;
extern int FB_H;
extern int FB_SIZE;
extern int TMP_SIZE;
extern int show_fps;
extern int down_scale;

int need_reload_bg = 0;
static int threading_mode = 0;
static struct _NDS_TEXTURE ntex[MAX_TEXTURE] = {0};

static int update_texture(void *chk, void *new, const void *pixels, int pitch)
{
    int cc = 0;

    for (cc=0; cc<MAX_TEXTURE; cc++) {
        if (ntex[cc].texture == chk) {
            ntex[cc].texture = new;
            ntex[cc].pixels = pixels;
            ntex[cc].pitch = pitch;
            return cc;
        }
    }
    return -1;
}

const void* get_pixels(void *chk)
{
    int cc = 0;

    for (cc=0; cc<MAX_TEXTURE; cc++) {
        if (ntex[cc].texture == chk) {
            return ntex[cc].pixels;
        }
    }
    return NULL;
}

static int get_pitch(void *chk)
{
    int cc = 0;

    for (cc=0; cc<MAX_TEXTURE; cc++) {
        if (ntex[cc].texture == chk) {
            return ntex[cc].pitch;
        }
    }
    return -1;
}

static void MMIYOO_WindowEvent(SDL_Renderer *renderer, const SDL_WindowEvent *event)
{
}

static int MMIYOO_CreateTexture(SDL_Renderer *renderer, SDL_Texture *texture)
{
    MMIYOO_TextureData *mmiyoo_texture = (MMIYOO_TextureData *)SDL_calloc(1, sizeof(*mmiyoo_texture));

    if(!mmiyoo_texture) {
        printf(PREFIX"Failed to create texture\n");
        return SDL_OutOfMemory();
    }

    mmiyoo_texture->width = texture->w;
    mmiyoo_texture->height = texture->h;
    mmiyoo_texture->format = texture->format;

    switch(mmiyoo_texture->format) {
    case SDL_PIXELFORMAT_RGB565:
        mmiyoo_texture->bits = 16;
        break;
    case SDL_PIXELFORMAT_ARGB8888:
        mmiyoo_texture->bits = 32;
        break;
    default:
        return -1;
    }

    mmiyoo_texture->pitch = mmiyoo_texture->width * SDL_BYTESPERPIXEL(texture->format);
    mmiyoo_texture->size = mmiyoo_texture->height * mmiyoo_texture->pitch;
    mmiyoo_texture->data = SDL_calloc(1, mmiyoo_texture->size);

    if(!mmiyoo_texture->data) {
        printf(PREFIX"Failed to create texture data\n");
        SDL_free(mmiyoo_texture);
        return SDL_OutOfMemory();
    }

    mmiyoo_texture->data = SDL_calloc(1, mmiyoo_texture->size);
    texture->driverdata = mmiyoo_texture;
    update_texture(NULL, texture, NULL, 0);
    GFX_Clear();
    return 0;
}

static int MMIYOO_LockTexture(SDL_Renderer *renderer, SDL_Texture *texture, const SDL_Rect *rect, void **pixels, int *pitch)
{
    MMIYOO_TextureData *mmiyoo_texture = (MMIYOO_TextureData*)texture->driverdata;

    *pixels = mmiyoo_texture->data;
    *pitch = mmiyoo_texture->pitch;
    return 0;
}

static int MMIYOO_UpdateTexture(SDL_Renderer *renderer, SDL_Texture *texture, const SDL_Rect *rect, const void *pixels, int pitch)
{
    update_texture(texture, texture, pixels, pitch);
    return 0;
}

static void MMIYOO_UnlockTexture(SDL_Renderer *renderer, SDL_Texture *texture)
{
    SDL_Rect rect = {0};
    MMIYOO_TextureData *mmiyoo_texture = (MMIYOO_TextureData*)texture->driverdata;

    rect.x = 0;
    rect.y = 0;
    rect.w = texture->w;
    rect.h = texture->h;
    MMIYOO_UpdateTexture(renderer, texture, &rect, mmiyoo_texture->data, mmiyoo_texture->pitch);
}

static void MMIYOO_SetTextureScaleMode(SDL_Renderer *renderer, SDL_Texture *texture, SDL_ScaleMode scaleMode)
{
}

static int MMIYOO_SetRenderTarget(SDL_Renderer *renderer, SDL_Texture *texture)
{
    return 0;
}

static int MMIYOO_QueueSetViewport(SDL_Renderer *renderer, SDL_RenderCommand *cmd)
{
    return 0;
}

static int MMIYOO_QueueDrawPoints(SDL_Renderer *renderer, SDL_RenderCommand *cmd, const SDL_FPoint *points, int count)
{
    return 0;
}

static int MMIYOO_QueueGeometry(SDL_Renderer *renderer, SDL_RenderCommand *cmd, SDL_Texture *texture,
                                const float *xy, int xy_stride, const SDL_Color *color, int color_stride, const float *uv, int uv_stride,
                                int num_vertices, const void *indices, int num_indices, int size_indices,
                                float scale_x, float scale_y)
{
    return 0;
}

static int MMIYOO_QueueFillRects(SDL_Renderer *renderer, SDL_RenderCommand *cmd, const SDL_FRect *rects, int count)
{
    return 0;
}

static int MMIYOO_QueueCopy(SDL_Renderer *renderer, SDL_RenderCommand *cmd, SDL_Texture *texture, const SDL_Rect *srcrect, const SDL_FRect *dstrect)
{
    if (evt.mode == MMIYOO_MOUSE_MODE) {
        threading_mode = 0;
        return My_QueueCopy(texture, get_pixels(texture), srcrect, dstrect);
    }
    
    threading_mode = 1;
    gfx.thread.texture = texture;
    gfx.thread.srt.x = srcrect->x;
    gfx.thread.srt.y = srcrect->y;
    gfx.thread.srt.w = srcrect->w;
    gfx.thread.srt.h = srcrect->h;
    gfx.thread.drt.x = dstrect->x;
    gfx.thread.drt.y = dstrect->y;
    gfx.thread.drt.w = dstrect->w;
    gfx.thread.drt.h = dstrect->h;
    return 0;
}

int My_QueueCopy(SDL_Texture *texture, const void *pixels, const SDL_Rect *srcrect, const SDL_FRect *dstrect)
{
    int pitch = 0;
    SDL_Rect dst = {dstrect->x, dstrect->y, dstrect->w, dstrect->h};
    SDL_Rect src = {srcrect->x, srcrect->y, srcrect->w, srcrect->h};

    pitch = get_pitch(texture);
    if ((pitch == 0) || (pixels == NULL)) {
        return 0;
    }

    evt.mouse.minx = dstrect->x;
    evt.mouse.miny = dstrect->y;
    evt.mouse.maxx = evt.mouse.minx + dstrect->w;
    evt.mouse.maxy = evt.mouse.miny + dstrect->h;
    if (0 && (evt.mode == MMIYOO_MOUSE_MODE)) {
        draw_pen(pixels, src.w, pitch);
    }

    dst.w = 640;
    dst.h = 480;
    GFX_Copy(pixels, src, dst, pitch, 0, E_MI_GFX_ROTATE_180);
    return 0;
}

static int MMIYOO_QueueCopyEx(SDL_Renderer *renderer, SDL_RenderCommand *cmd, SDL_Texture *texture,
    const SDL_Rect *srcrect, const SDL_FRect *dstrect, const double angle, const SDL_FPoint *center, const SDL_RendererFlip flip)
{
    return 0;
}

static int MMIYOO_RunCommandQueue(SDL_Renderer *renderer, SDL_RenderCommand *cmd, void *vertices, size_t vertsize)
{
    return 0;
}

static int MMIYOO_RenderReadPixels(SDL_Renderer *renderer, const SDL_Rect *rect, Uint32 pixel_format, void *pixels, int pitch)
{
    return SDL_Unsupported();
}

static void MMIYOO_RenderPresent(SDL_Renderer *renderer)
{
    if (threading_mode > 0) {
        gfx.action = GFX_ACTION_FLIP;
    }
    else {
        GFX_Flip();
    }
}

static void MMIYOO_DestroyTexture(SDL_Renderer *renderer, SDL_Texture *texture)
{
    MMIYOO_TextureData *mmiyoo_texture = (MMIYOO_TextureData*)texture->driverdata;

    if (mmiyoo_texture) {
        update_texture(texture, NULL, NULL, 0);
        if (mmiyoo_texture->data) {
            SDL_free(mmiyoo_texture->data);
        }
        SDL_free(mmiyoo_texture);
        texture->driverdata = NULL;
    }
}

static void MMIYOO_DestroyRenderer(SDL_Renderer *renderer)
{
    MMIYOO_RenderData *data = (MMIYOO_RenderData *)renderer->driverdata;

    if(data) {
        if(!data->initialized) {
            return;
        }

        data->initialized = SDL_FALSE;
        SDL_free(data);
    }
    SDL_free(renderer);
}

static int MMIYOO_SetVSync(SDL_Renderer *renderer, const int vsync)
{
    return 0;
}

SDL_Renderer *MMIYOO_CreateRenderer(SDL_Window *window, Uint32 flags)
{
    int pixelformat = 0;
    SDL_Renderer *renderer = NULL;
    MMIYOO_RenderData *data = NULL;

    renderer = (SDL_Renderer *) SDL_calloc(1, sizeof(*renderer));
    if(!renderer) {
        printf(PREFIX"Failed to create render\n");
        SDL_OutOfMemory();
        return NULL;
    }

    data = (MMIYOO_RenderData *) SDL_calloc(1, sizeof(*data));
    if(!data) {
        printf(PREFIX"Failed to create render data\n");
        MMIYOO_DestroyRenderer(renderer);
        SDL_OutOfMemory();
        return NULL;
    }

    renderer->WindowEvent = MMIYOO_WindowEvent;
    renderer->CreateTexture = MMIYOO_CreateTexture;
    renderer->UpdateTexture = MMIYOO_UpdateTexture;
    renderer->LockTexture = MMIYOO_LockTexture;
    renderer->UnlockTexture = MMIYOO_UnlockTexture;
    renderer->SetTextureScaleMode = MMIYOO_SetTextureScaleMode;
    renderer->SetRenderTarget = MMIYOO_SetRenderTarget;
    renderer->QueueSetViewport = MMIYOO_QueueSetViewport;
    renderer->QueueSetDrawColor = MMIYOO_QueueSetViewport;
    renderer->QueueDrawPoints = MMIYOO_QueueDrawPoints;
    renderer->QueueDrawLines = MMIYOO_QueueDrawPoints;
    renderer->QueueGeometry = MMIYOO_QueueGeometry;
    renderer->QueueFillRects = MMIYOO_QueueFillRects;
    renderer->QueueCopy = MMIYOO_QueueCopy;
    renderer->QueueCopyEx = MMIYOO_QueueCopyEx;
    renderer->RunCommandQueue = MMIYOO_RunCommandQueue;
    renderer->RenderReadPixels = MMIYOO_RenderReadPixels;
    renderer->RenderPresent = MMIYOO_RenderPresent;
    renderer->DestroyTexture = MMIYOO_DestroyTexture;
    renderer->DestroyRenderer = MMIYOO_DestroyRenderer;
    renderer->SetVSync = MMIYOO_SetVSync;
    renderer->info = MMIYOO_RenderDriver.info;
    renderer->info.flags = (SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);
    renderer->driverdata = data;
    renderer->window = window;

    if(data->initialized != SDL_FALSE) {
        return 0;
    }
    data->initialized = SDL_TRUE;

    if(flags & SDL_RENDERER_PRESENTVSYNC) {
        data->vsync = SDL_TRUE;
    }
    else {
        data->vsync = SDL_FALSE;
    }

    pixelformat = SDL_GetWindowPixelFormat(window);
    switch(pixelformat) {
    case SDL_PIXELFORMAT_RGB565:
        data->bpp = 2;
        break;
    case SDL_PIXELFORMAT_ARGB8888:
        data->bpp = 4;
        break;
    }
    return renderer;
}

SDL_RenderDriver MMIYOO_RenderDriver = {
    .CreateRenderer = MMIYOO_CreateRenderer,
    .info = {
        .name = "MMIYOO",
        .flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE,
        .num_texture_formats = 2,
        .texture_formats = {
            [0] = SDL_PIXELFORMAT_RGB565, [2] = SDL_PIXELFORMAT_ARGB8888,
        },
        .max_texture_width = 800,
        .max_texture_height = 600,
    }
};

#endif

