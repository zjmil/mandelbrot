
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>

#include <SDL.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

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

typedef struct Graph {
    float centerx, centery;
    float xscale, yscale;
} Graph;

void graph_init_default(Graph *g)
{
    g->centerx = 0.0f;
    g->centery = 0.0f;
    g->xscale = 1.0f / 400.0f;
    g->yscale = 1.0f / 400.0f;
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

void render_mandelbrot(SDL_Renderer *renderer, SDL_Color *colors, int ncolors, Mandelbrot *m, Graph *g)
{
    int width = 0, height = 0;
    SDL_GetRendererOutputSize(renderer, &width, &height);

    float tx = g->centerx - (g->xscale * width) / 2.0f;
    float ty = g->centery + (g->yscale * height) / 2.0f;

    // not doing any blending
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

    for (int py = 0; py < height; py++) {
        float y0 = ty - py * g->yscale;

        for (int px = 0; px < width; px++) {
            float x0 = tx + px * g->xscale;

            int iterations = mandelbrot_iterations(m, x0, y0);

            SDL_Color c = colors[iterations % ncolors];

            SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, 0xff);
            SDL_RenderDrawPoint(renderer, px, py);
        }
    }
}

typedef struct Context {
    int width, height;
    Graph graph;
    Mandelbrot m;
    uint32_t window_id;
    SDL_Color *colors;
    size_t ncolors;
    SDL_Renderer *renderer;
    bool rerender;
} Context;

bool handle_events(Context *ctx)
{
    Graph *graph = &ctx->graph;
    Mandelbrot *m = &ctx->m;

    SDL_Event event;
    bool running = true;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
            running = false;
            break;
        case SDL_WINDOWEVENT:
            if (event.window.windowID == ctx->window_id) {
                switch (event.window.event) {
                case SDL_WINDOWEVENT_RESIZED: {
                    int new_width = event.window.data1;
                    int new_height = event.window.data2;
                    SDL_Log("New window size: width=%d, height=%d\n", new_width, new_height);

                    float width_change = (float)new_width / (float)ctx->width;
                    float height_change = (float)new_height / (float)ctx->height;

                    graph->xscale *= width_change;
                    graph->yscale *= height_change;

                    ctx->width = new_width;
                    ctx->height = new_height;

                    ctx->rerender = true;
                    break;
                }
                }
            }
        case SDL_KEYUP:
            switch (event.key.keysym.sym) {
            case SDLK_q:
                running = false;
                break;
            // scaling
            case SDLK_EQUALS:
                if (event.key.keysym.mod & KMOD_SHIFT) {
                    graph->xscale *= 0.9f;
                    graph->yscale *= 0.9f;
                    ctx->rerender = true;
                }
                break;
            case SDLK_MINUS:
                graph->xscale *= 10.0f / 9.0f;
                graph->yscale *= 10.0f / 9.0f;
                ctx->rerender = true;
                break;
            // moving
            case SDLK_UP:
                graph->centery += graph->yscale * 10.0f;
                ctx->rerender = true;
                break;
            case SDLK_DOWN:
                graph->centery -= graph->yscale * 10.0f;
                ctx->rerender = true;
                break;
            case SDLK_LEFT:
                graph->centerx -= graph->xscale * 10.0f;
                ctx->rerender = true;
                break;
            case SDLK_RIGHT:
                graph->centery += graph->xscale * 10.0f;
                ctx->rerender = true;
                break;
            // reset
            case SDLK_r:
                graph_init_default(graph);
                ctx->rerender = true;
                break;
            }
            break;
        }
    }

    if (ctx->rerender) {
        ctx->rerender = false;
        printf("Rendering mandelbrot\n");
        render_mandelbrot(ctx->renderer, ctx->colors, ctx->ncolors, m, graph);

        // show
        SDL_RenderPresent(ctx->renderer);
    }

    return running;
}

void main_loop(Context *ctx)
{
    while (handle_events(ctx)) {}
}

#ifdef __EMSCRIPTEN__

void handle_events_wrapper(void *arg)
{
    bool running = handle_events((Context *)arg);
    if (!running) {
        emscripten_cancel_main_loop();
    }
}

void em_loop(Context *ctx)
{
    emscripten_set_main_loop_arg(handle_events_wrapper, ctx, -1, 1);
}

#endif

int main(void)
{
    Context ctx;
    ctx.width = 700;
    ctx.height = 400;

    if (SDL_Init(SDL_INIT_VIDEO)) {
        fatal("Failed initializing SDL: %s\n", SDL_GetError());
    }

    SDL_Window *window = SDL_CreateWindow(
            "Mandelbrot",
            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
            ctx.width, ctx.height,
            SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI
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
    mandelbrot_init_default(&ctx.m);
    graph_init_default(&ctx.graph);
    ctx.window_id = SDL_GetWindowID(window);
    ctx.colors = colors;
    ctx.ncolors = ARRAY_SIZE(colors);
    ctx.renderer = renderer;
    ctx.rerender = true;

    // clear the screen
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0xff);
    SDL_RenderClear(renderer);

#ifdef __EMSCRIPTEN__
    em_loop(&ctx);
#else
    main_loop(&ctx);
#endif

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();
    return EXIT_SUCCESS;
}
