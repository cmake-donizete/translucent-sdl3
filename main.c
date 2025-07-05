#include <main.h>

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

static void parse_args(int argc, char *argv[])
{
    int opt, index;

    struct option long_options[] = {
        {"window_scale", required_argument, 0, 'w'},
        {"image_scale", required_argument, 0, 'i'},
        {"path", required_argument, 0, 'p'},
        {0, 0, 0, 0},
    };

    while ((opt = getopt_long(argc, argv, "w:i:p:", long_options, &index)) != -1)
    {
        switch (opt)
        {
        case 'w':
        {
            args.window_scale = atof(optarg);
            break;
        }

        case 'i':
        {
            args.image_scale = atof(optarg);
            break;
        }

        case 'p':
        {
            strncpy(args.path, optarg, sizeof(args.path));
            break;
        }

        default:
        {
            fprintf(stderr, "Unknown argument: %d", optopt);
            exit(EXIT_FAILURE);
            break;
        }
        }
    }
}

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    parse_args(argc, argv);

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

    if (!(surface = IMG_Load(args.path)))
    {
        SDL_Log("Couldn't load image: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!SDL_CreateWindowAndRenderer(
            "translucent",
            surface->w * args.window_scale,
            surface->h * args.window_scale,
            SDL_WINDOW_ALWAYS_ON_TOP | SDL_WINDOW_TRANSPARENT | SDL_WINDOW_RESIZABLE,
            &state.window,
            &state.renderer))
    {
        SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!(state.texture = SDL_CreateTextureFromSurface(state.renderer, surface)))
    {
        SDL_Log("Couldn't create texture: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    SDL_DestroySurface(surface);

    state.f_rect.w = state.texture->w * args.image_scale;
    state.f_rect.h = state.texture->h * args.image_scale;

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    SDL_Window *window = state.window;
    SDL_Texture *texture = state.texture;
    SDL_Point *mouse_movement = &state.mouse_movement;
    SDL_FRect *f_rect = &state.f_rect;

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

        if (scancode == SDL_SCANCODE_R)
        {
            state.scale = 1.0f;
            state.f_rect = (SDL_FRect){
                .x = 0,
                .y = 0,
                .w = texture->w * args.image_scale,
                .h = texture->h * args.image_scale,
            };
            SDL_SetWindowSize(window,
                              texture->w * args.window_scale,
                              texture->h * args.window_scale);
        }
    }

    if (event->type == SDL_EVENT_MOUSE_WHEEL)
    {
        state.scale += (event->wheel.y > .0f) ? +.1f : -.1f;

        f_rect->w = texture->w * state.scale * args.image_scale;
        f_rect->h = texture->h * state.scale * args.image_scale;
    }

    if (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN && !state.is_dragging)
    {
        mouse_movement->x = event->motion.x;
        mouse_movement->y = event->motion.y;

        state.is_dragging = true;
    }

    if (event->type == SDL_EVENT_MOUSE_MOTION)
    {
        if (state.is_dragging)
        {
            f_rect->x += event->motion.x - mouse_movement->x;
            f_rect->y += event->motion.y - mouse_movement->y;
        }

        mouse_movement->x = event->motion.x;
        mouse_movement->y = event->motion.y;
    }

    if (event->type == SDL_EVENT_MOUSE_BUTTON_UP)
    {
        state.is_dragging = false;
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate)
{
    SDL_RenderClear(state.renderer);
    SDL_RenderTexture(state.renderer, state.texture, NULL, &state.f_rect);
    SDL_RenderPresent(state.renderer);

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
}