#include <main.h>

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

static void parse_args(int argc, char *argv[])
{
    int opt, index;

    struct option long_options[] = {
        {"window_scale", required_argument, 0, 'w'},
        {"image_scale", required_argument, 0, 'i'},
        {"filename", required_argument, 0, 'f'},
        {0, 0, 0, 0},
    };

    while ((opt = getopt_long(argc, argv, "w:i:f:", long_options, &index)) != -1)
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

        case 'f':
        {
            strncpy(args.filename, optarg, sizeof(args.filename));
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

    if (!(surface = IMG_Load(args.filename)))
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

    state.window_scale = args.window_scale;
    state.image_scale = args.image_scale;

    state.rect_dest.w = state.texture->w;
    state.rect_dest.h = state.texture->h;

    SDL_SetWindowOpacity(
        state.window,
        state.opacity);
    SDL_SetRenderScale(
        state.renderer,
        state.image_scale,
        state.image_scale);
    SDL_SetTextureScaleMode(
        state.texture,
        SDL_SCALEMODE_NEAREST);

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    SDL_Window *window = state.window;
    SDL_Texture *texture = state.texture;
    SDL_FRect *rect_dest = &state.rect_dest;

    if (event->type == SDL_EVENT_QUIT)
    {
        return SDL_APP_SUCCESS;
    }

    if (event->type == SDL_EVENT_KEY_DOWN)
    {
        SDL_Scancode scancode = event->key.scancode;
        if (scancode >= SDL_SCANCODE_1 && scancode <= SDL_SCANCODE_0)
        {
            uint8_t number = ((scancode + 1) - SDL_SCANCODE_1) % 10;
            state.tmp_scale *= 10;
            state.tmp_scale += number;
        }

        if (scancode == SDL_SCANCODE_R)
        {
            state.window_scale = args.window_scale;
            state.image_scale = args.image_scale;
            state.opacity = args.opacity;
            state.rect_dest.x = 0;
            state.rect_dest.y = 0;

            SDL_SetWindowOpacity(
                window,
                state.opacity);
            SDL_SetRenderScale(
                state.renderer,
                state.image_scale,
                state.image_scale);
            SDL_SetWindowSize(
                window,
                texture->w * args.window_scale,
                texture->h * args.window_scale);
        }

        switch (scancode)
        {
        case SDL_SCANCODE_RIGHT:
            rect_dest->x += 1;
            break;
        case SDL_SCANCODE_LEFT:
            rect_dest->x += -1;
            break;
        case SDL_SCANCODE_DOWN:
            rect_dest->y += 1;
            break;
        case SDL_SCANCODE_UP:
            rect_dest->y -= 1;
            break;
        }

        if (!state.is_scaling)
        {
            if (scancode == SDL_SCANCODE_W)
            {
                state.is_scaling = SDL_SCANCODE_W;
            }
            else if (scancode == SDL_SCANCODE_I)
            {
                state.is_scaling = SDL_SCANCODE_I;
            }
            state.tmp_scale = 0;
        }
        else
        {
            if (scancode == SDL_SCANCODE_RETURN)
            {
                if (state.is_scaling == SDL_SCANCODE_W)
                {
                    state.window_scale = state.tmp_scale / 100.0f;
                }
                if (state.is_scaling == SDL_SCANCODE_I)
                {
                    state.image_scale = state.tmp_scale / 100.0f;
                }

                SDL_SetWindowSize(
                    window,
                    texture->w * state.window_scale,
                    texture->h * state.window_scale);
                SDL_SetRenderScale(
                    state.renderer,
                    state.image_scale,
                    state.image_scale);

                state.tmp_scale = 0;
                state.is_scaling = SDL_SCANCODE_UNKNOWN;
            }
            if (scancode == SDL_SCANCODE_ESCAPE)
            {
                state.tmp_scale = 0;
                state.is_scaling = SDL_SCANCODE_UNKNOWN;
                return SDL_APP_CONTINUE;
            }
        }

        if (scancode == SDL_SCANCODE_ESCAPE)
        {
            return SDL_APP_SUCCESS;
        }
    }

    if (event->type == SDL_EVENT_MOUSE_WHEEL)
    {
        state.opacity += (event->wheel.y > .0f) ? +.1f : -.1f;
        state.opacity = SDL_clamp(state.opacity, 0.0f, 1.0f);
        SDL_SetWindowOpacity(window, state.opacity);
    }

    if (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN && !state.is_dragging)
    {
        state.is_dragging = true;
    }

    if (event->type == SDL_EVENT_MOUSE_MOTION && state.is_dragging)
    {
        // since our renderer is scaled we need to fix the event before usage
        SDL_ConvertEventToRenderCoordinates(state.renderer, event);
        rect_dest->x += event->motion.xrel;
        rect_dest->y += event->motion.yrel;
    }

    if (event->type == SDL_EVENT_MOUSE_BUTTON_UP)
    {
        state.is_dragging = false;

        // round our drag and drop to be pixel perfect
        rect_dest->x = round(rect_dest->x);
        rect_dest->y = round(rect_dest->y);
    }

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate)
{
    SDL_RenderClear(state.renderer);
    SDL_RenderTexture(state.renderer, state.texture, NULL, &state.rect_dest);
    SDL_RenderPresent(state.renderer);

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
}