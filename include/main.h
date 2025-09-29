#define SDL_MAIN_USE_CALLBACKS 1

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>

struct args
{
    float window_scale;
    float image_scale;
    float opacity;
    char filename[256];
};

struct state
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    SDL_FRect rect_dest;

    bool is_dragging;
    float window_scale;
    float image_scale;
    float opacity;
    uint8_t is_scaling;
    uint16_t tmp_scale;
};

static struct args args = {
    .window_scale = 1.0f,
    .image_scale = 1.0f,
    .opacity = .5f,
};

static struct state state = {
    .window_scale = 1.0f,
    .image_scale = 1.0f,
    .opacity = .5f,
    .rect_dest = {
        .x = 0,
        .y = 0,
        .w = 0,
        .h = 0,
    },
    .is_scaling = SDL_SCANCODE_UNKNOWN,
    .tmp_scale = 0,
};