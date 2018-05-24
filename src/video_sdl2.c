#include <SDL2/SDL.h>

#if WIN32
#define BITMAP __win_bitmap
#include <windows.h>
#undef BITMAP
#endif

#include <stdio.h>
#include "arc.h"
#include "plat_video.h"
#include "vidc.h"
#include "video_sdl2.h"

static SDL_Texture *texture = NULL;
static SDL_Renderer *renderer = NULL;
SDL_Window *sdl_main_window = NULL;
static SDL_Rect texture_rect;

int video_renderer_init(void *main_window)
{
        SDL_Rect screen_rect;
        
        rpclog("video_renderer_init()\n");
        SDL_SetHint(SDL_HINT_WINDOWS_DISABLE_THREAD_NAMING, "1");
        SDL_Init(SDL_INIT_EVERYTHING);

        rpclog("create SDL window\n");
        if (main_window == NULL)
        {
                sdl_main_window = SDL_CreateWindow(
                        "Arculator",
                        SDL_WINDOWPOS_CENTERED,
                        SDL_WINDOWPOS_CENTERED,
                        768,
                        576,
                        0
                );
        }
        else
        {
                sdl_main_window = SDL_CreateWindowFrom(main_window);
        }
        screen_rect.w = screen_rect.h = 2048;

        if (!sdl_main_window)
        {
                char message[200];
                sprintf(message,
                                "SDL_CreateWindowFrom could not be created! Error: %s\n",
                                SDL_GetError());
#if WIN32
                MessageBox(main_window, message, "SDL Error", MB_OK);
#else
                printf("%s", message);
#endif                
                return SDL_FALSE;
        }

        rpclog("create SDL renderer\n");
        renderer = SDL_CreateRenderer(sdl_main_window, -1, SDL_RENDERER_ACCELERATED);

        if (!renderer)
        {
                char message[200];
                sprintf(message,
                                "SDL window could not be created! Error: %s\n",
                                SDL_GetError());
#if WIN32
                MessageBox(main_window, message, "SDL Error", MB_OK);
#else
                printf("%s", message);
#endif                
                return SDL_FALSE;
        }

        rpclog("create texture\n");
        texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
                        SDL_TEXTUREACCESS_STREAMING,
                        screen_rect.w, screen_rect.h);

        rpclog("video initialized\n");
        return SDL_TRUE;
}

void video_renderer_close()
{
        if (texture)
        {
                SDL_DestroyTexture(texture);
                texture = NULL;
        }
        if (renderer)
        {
                SDL_DestroyRenderer(renderer);
                renderer = NULL;
        }
}

/*Update display texture from memory bitmap src.*/
void video_renderer_update(BITMAP *src, int src_x, int src_y, int dest_x, int dest_y, int w, int h)
{
        LOG_VIDEO_FRAMES("video_renderer_update: src=%i,%i dest=%i,%i size=%i,%i\n", src_x,src_y, dest_x,dest_y, w,h);
        texture_rect.x = dest_x;
        texture_rect.y = dest_y;
        texture_rect.w = w;
        texture_rect.h = h;
        
        if (src_x < 0)
        {
                texture_rect.w += src_x;
                src_x = 0;
        }
        if (src_x > 2047)
                return;
        if ((src_x + texture_rect.w) > 2047)
                texture_rect.w = 2048 - src_x;

        if (src_y < 0)
        {
                texture_rect.h += src_y;
                src_y = 0;
        }
        if (src_y > 2047)
                return;
        if ((src_y + texture_rect.h) > 2047)
                texture_rect.h = 2048 - src_y;
        
        if (texture_rect.x < 0)
        {
                texture_rect.w += texture_rect.x;
                texture_rect.x = 0;
        }
        if (texture_rect.x > 2047)
                return;
        if ((texture_rect.x + texture_rect.w) > 2047)
                texture_rect.w = 2048 - texture_rect.x;

        if (texture_rect.y < 0)
        {
                texture_rect.h += texture_rect.y;
                texture_rect.y = 0;
        }
        if (texture_rect.y > 2047)
                return;
        if ((texture_rect.y + texture_rect.h) > 2047)
                texture_rect.h = 2048 - texture_rect.y;

        LOG_VIDEO_FRAMES("SDL_UpdateTexture (%d, %d)+(%d, %d) from src (%d, %d) w %d\n",
                texture_rect.x, texture_rect.y,
                texture_rect.w, texture_rect.h,
                src_x, src_y, src->w);
        SDL_UpdateTexture(texture, &texture_rect, &((uint32_t *)src->dat)[src_y * src->w + src_x], src->w * 4);
}

/*Render display texture to video window.*/
void video_renderer_present(int src_x, int src_y, int src_w, int src_h)
{
        LOG_VIDEO_FRAMES("video_renderer_present: %d,%d + %d,%d\n", src_x, src_y, src_w, src_h);

        SDL_Rect window_rect;

        SDL_GetWindowSize(sdl_main_window, &window_rect.w, &window_rect.h);
        window_rect.x = 0;
        window_rect.y = 0;

        texture_rect.x = src_x;
        texture_rect.y = src_y;
        texture_rect.w = src_w;
        texture_rect.h = src_h;

        /*Clear the renderer backbuffer*/
        SDL_RenderClear(renderer);
/*rpclog("Present %i,%i %i,%i -> %i,%i %i,%i\n", texture_rect.x, texture_rect.y, texture_rect.w, texture_rect.h,
                                                window_rect.x, window_rect.y, window_rect.w, window_rect.h);*/
        /*Copy texture to backbuffer*/
        SDL_RenderCopy(renderer, texture, &texture_rect, &window_rect);
        /*Present backbuffer to window*/
        SDL_RenderPresent(renderer);
}
