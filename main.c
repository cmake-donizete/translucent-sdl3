
#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static SDL_Texture *texture = NULL;
static SDL_FRect texture_rect;
static SDL_Point mouse_movement;
static bool is_dragging = false;

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    SDL_Surface *surface = NULL;

    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!SDL_SetHint(SDL_HINT_MAIN_CALLBACK_RATE, "60"))
    {
        SDL_Log("Couldn't set framerate: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!(surface = IMG_Load("sample.jpeg")))
    {
        SDL_Log("Couldn't load image: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!SDL_CreateWindowAndRenderer(
            "translucent",
            surface->w,
            surface->h,
            SDL_WINDOW_ALWAYS_ON_TOP | SDL_WINDOW_TRANSPARENT,
            &window,
            &renderer))
    {
        SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!(texture = SDL_CreateTextureFromSurface(renderer, surface)))
    {
        SDL_Log("Couldn't create texture: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    SDL_DestroySurface(surface);

    texture_rect.w = texture->w;
    texture_rect.h = texture->h;

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    if (event->type == SDL_EVENT_QUIT)
    {
        return SDL_APP_SUCCESS;
    }

    if (event->type == SDL_EVENT_KEY_DOWN)
    {
        SDL_Scancode scancode = event->key.scancode;
        if (scancode >= SDL_SCANCODE_1 && scancode <= SDL_SCANCODE_0)
        {
            float alpha = (scancode - SDL_SCANCODE_1) / 10.0f;
            SDL_SetWindowOpacity(window, alpha);
        }
    }

    if (event->type == SDL_EVENT_MOUSE_WHEEL)
    {
        float width = texture_rect.w;
        float height = texture_rect.h;

        float scale = (event->wheel.y > .0f) ? +.1f : -.1f;

        texture_rect.w += width * scale;
        texture_rect.h += height * scale;

        texture_rect.x -= (texture_rect.w - width) / 2.0f;
        texture_rect.y -= (texture_rect.h - height) / 2.0f;
    }

    if (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN && !is_dragging)
    {
        mouse_movement.x = event->motion.x;
        mouse_movement.y = event->motion.y;

        is_dragging = true;
    }

    if (event->type == SDL_EVENT_MOUSE_MOTION && is_dragging)
    {
        texture_rect.x += event->motion.x - mouse_movement.x;
        texture_rect.y += event->motion.y - mouse_movement.y;

        mouse_movement.x = event->motion.x;
        mouse_movement.y = event->motion.y;
    }

    if (event->type == SDL_EVENT_MOUSE_BUTTON_UP)
    {
        is_dragging = false;
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate)
{
    SDL_RenderClear(renderer);
    SDL_RenderTexture(renderer, texture, NULL, &texture_rect);
    SDL_RenderPresent(renderer);

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
}