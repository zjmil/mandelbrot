
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>

#include <SDL.h>
#include <SDL2_gfxPrimitives.h>

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

typedef struct Bounds {
    float x0, x1;
    float y0, y1;
} Bounds;

float bounds_xrange(Bounds *b)
{
    return b->x1 - b->x0;
}

float bounds_yrange(Bounds *b)
{
    return b->y1 - b->y0;
}

typedef struct Mandelbrot {
    int max_iterations;
    int max_periods;
} Mandelbrot;

void mandelbrot_init_default(Mandelbrot *m)
{
    m->max_iterations = 1000;
    m->max_periods = 20;
}

int mandelbrot_iterations(Mandelbrot *m, float x0, float y0)
{
    float x = 0.0, y = 0.0, x2 = 0.0, y2 = 0.0;
    int iterations = 0;

    float xold = 0.0, yold = 0.0;
    int period = 0;

    while ((x2 + y2) <= 4.0 && iterations < m->max_iterations) {
        y = 2*x*y + y0;
        x = x2 - y2 + x0;
        x2 = x*x;
        y2 = y*y;

        iterations += 1;

        if (approx(x, xold) && approx(y, yold)) {
            iterations = m->max_iterations;
            break;
        }

        period += 1;
        if (period > m->max_periods) {
            period = 0;
            xold = x;
            yold = y;
        }
    }

    return iterations;
}

void render_mandelbrot(SDL_Renderer *renderer, int width, int height, SDL_Color *colors, int ncolors, Mandelbrot *m, Bounds *bounds)
{
    float xdelta = bounds_xrange(bounds) / width;
    float ydelta = bounds_yrange(bounds) / height;

    for (int py = 0; py < height; py++) {
        float y0 = bounds->y0 + py * ydelta;

        for (int px = 0; px < width; px++) {
            float x0 = bounds->x0 + px * xdelta;

            int iterations = mandelbrot_iterations(m, x0, y0);

            SDL_Color c = colors[iterations % ncolors];
            pixelRGBA(renderer, px, py, c.r, c.g, c.b, 0xff);
        }
    }
}

int main(void)
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

    Mandelbrot m;
    mandelbrot_init_default(&m);

    Bounds bounds = {
        .x0 = -2.5,
        .x1 = 1.0,
        .y0 = -1.0,
        .y1 = 1.0
    };


    // clear the screen
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0xff);
    SDL_RenderClear(renderer);

    // TODO: future add movement and respond to window size changes, rerender accordingly
    render_mandelbrot(renderer, width, height, colors, ncolors, &m, &bounds);

    // show
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

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();
    return EXIT_SUCCESS;
}
