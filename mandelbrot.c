
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>

#include <SDL.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

void fatal(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, fmt, args);

    va_end(args);

    exit(EXIT_FAILURE);
}

bool approx(float a, float b)
{
    return fabsf(a - b) < 1.0e-7;
}

void calc_iterations(SDL_Color *pixels, int width, int height, SDL_Color *colors, int ncolors)
{
    int max_iterations = 1000, max_periods = 20;
    float xmin = -2.5, xmax = 1.0, ymin = -1.0, ymax = 1.0;

    float xrange = xmax - xmin, yrange = ymax - ymin;
    float xdelta = xrange / width, ydelta = yrange / height;

    for (int py = 0; py < height; py++) {
        float y0 = ymin + py * ydelta;

        for (int px = 0; px < width; px++) {
            float x0 = xmin + px * xdelta;

            float x = 0.0, y = 0.0, x2 = 0.0, y2 = 0.0;
            int iteration = 0;

            float xold = 0.0, yold = 0.0;
            int period = 0;

            while ((x2 + y2) <= 4.0 && iteration < max_iterations) {
                y = 2*x*y + y0;
                x = x2 - y2 + x0;
                x2 = x*x;
                y2 = y*y;

                iteration += 1;

                if (approx(x, xold) && approx(y, yold)) {
                    iteration = max_iterations;
                    break;
                }

                period += 1;
                if (period > max_periods) {
                    period = 0;
                    xold = x;
                    yold = y;
                }
            }

            int pidx = py * width + px;
            pixels[pidx] = colors[iteration % ncolors];
        }
    }
}


int main(int argc, char *argv[])
{
    int width = 700, height = 400;

    if (SDL_Init(SDL_INIT_VIDEO)) {
        fatal("Failed initializing SDL: %s\n", SDL_GetError());
    }

    SDL_Window *window = SDL_CreateWindow(
            "Mandelbrot",
            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
            width, height,
            SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    );
    if (!window) {
        fatal("Failed to create window: %s", SDL_GetError());
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        fatal("Failed to create renderer: %s", SDL_GetError());
    }

    SDL_Texture *texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_ABGR8888,
        SDL_TEXTUREACCESS_STREAMING,
        width, height
    );
    if (!texture) {
        fatal("Failed to create texture: %s", SDL_GetError());
    }

    SDL_Color colors[] = {{66, 30, 15, 255},
                          {25, 7, 26, 255},
                          {9, 1, 47, 255},
                          {4, 4, 73, 255},
                          {0, 7, 100, 255},
                          {12, 44, 138, 255},
                          {24, 82, 177, 255},
                          {57, 125, 209, 255},
                          {134, 181, 229, 255},
                          {211, 236, 248, 255},
                          {241, 233, 191, 255},
                          {248, 201, 95, 255},
                          {255, 170, 0, 255},
                          {204, 128, 0, 255},
                          {153, 87, 0, 255},
                          {106, 52, 3, 255}};
    size_t ncolors = ARRAY_SIZE(colors);

    SDL_Color *pixels = calloc(sizeof(*pixels), width * height);

    // clear the screen
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0xff);
    SDL_RenderClear(renderer);

    // TODO: future add movement and respond to window size changes, rerender accordingly
    calc_iterations(pixels, width, height, colors, ncolors);

    // render the set
    SDL_UpdateTexture(texture, NULL, pixels, width * 4);
    SDL_RenderCopy(renderer, texture, NULL, NULL);

    SDL_RenderPresent(renderer);

    SDL_Event event;
    uint32_t sleep_ms = 100;
    bool running = true;
    while (running) {
        // process events
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT:
                running = false;
                break;
            case SDL_KEYUP:
                switch (event.key.keysym.sym) {
                case SDLK_q:
                    running = false;
                    break;
                }
                break;
            default:
                break;
            }
        }

        SDL_Delay(sleep_ms);
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    free(pixels);

    SDL_Quit();
    return EXIT_SUCCESS;
}
