#define SDL_MAIN_USE_CALLBACKS 1

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>

struct args
{
    float window_scale;
    float image_scale;
    char filename[256];
};

struct state
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;

    SDL_FRect f_rect;
    SDL_Point mouse_movement;

    bool is_dragging;
    float scale;
};

static struct args args = {
    .window_scale = 1.0f,
    .image_scale = 1.0f,
};

static struct state state = {
    .scale = 1.0f
};