
#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>

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

bool approxf(float a, float b)
{
    return fabsf(a - b) < 1.0e-7;
}

typedef struct Point2D {
    float x, y;
} Point2D;

typedef struct Graph {
    Point2D center;
    float scale;  // TODO: add different xscale and yscale for resize events
} Graph;

void Graph_init_default(Graph *g)
{
    g->center = (Point2D){.x = 0.0f, .y = 0.0f};
    g->scale = 1.0f / 400.0f;
}

void Graph_pan_right(Graph *g, float amt)
{
    g->center.x += g->scale * amt;
}

void Graph_pan_left(Graph *g, float amt)
{
    g->center.x -= g->scale * amt;
}

void Graph_pan_up(Graph *g, float amt)
{
    g->center.y += g->scale * amt;
}

void Graph_pan_down(Graph *g, float amt)
{
    g->center.y -= g->scale * amt;
}

void Graph_zoom(Graph *g, float amt)
{
    g->scale *= amt;
}

typedef struct Mandelbrot {
    int max_iterations;
    int max_periods;
    int width, height;
    int *iterations;
} Mandelbrot;

void Mandelbrot_init_default(Mandelbrot *m, int width, int height)
{
    m->max_iterations = 1000;
    m->max_periods = 20;
    m->iterations = malloc(sizeof(int) * width * height);
    m->width = width;
    m->height = height;
}

int Mandelbrot_idx(Mandelbrot *m, int x, int y)
{
    assert(x < m->width);
    assert(y < m->height);
    return y * m->width + x;
}

void Mandelbrot_set_iterations(Mandelbrot *m, int x, int y, int iters)
{
    m->iterations[Mandelbrot_idx(m, x, y)] = iters;
}

int Mandelbrot_get_iterations(Mandelbrot *m, int x, int y)
{
    return m->iterations[Mandelbrot_idx(m, x, y)];
}

void Mandelbrot_free(Mandelbrot *m)
{
    free(m->iterations);
}

int mandelbrot_point_iterations(float x0, float y0, int max_iterations, int max_periods)
{
    float x = 0.0f, y = 0.0f, x2 = 0.0f, y2 = 0.0f;
    int iterations = 0;

    float oldx = 0.0f, oldy = 0.0f;
    int period = 0;

    while ((x2 + y2) <= 4.0 && iterations < max_iterations) {
        y = 2 * x * y + y0;
        x = x2 - y2 + x0;
        x2 = x * x;
        y2 = y * y;

        iterations += 1;

        if (approxf(x, oldx) && approxf(y, oldy)) {
            iterations = max_iterations;
            break;
        }

        period += 1;
        if (period > max_periods) {
            period = 0;
            oldx = x;
            oldy = y;
        }
    }

    return iterations;
}

void mandelbrot_iterations_range(Mandelbrot *m, Graph *g, int startx, int starty, int endx, int endy)
{
    float tx = g->center.x - (g->scale * m->width) / 2.0f;
    float ty = g->center.y + (g->scale * m->height) / 2.0f;

    for (int py = starty; py < endy; py++) {
        float y0 = ty - g->scale * py;

        for (int px = startx; px < endx; px++) {
            float x0 = tx + g->scale * px;

            int iters = mandelbrot_point_iterations(x0, y0, m->max_iterations, m->max_periods);

            Mandelbrot_set_iterations(m, px, py, iters);
        }
    }
}

typedef struct MandelbrotThreadContext {
    Mandelbrot *m;
    Graph *g;
    int startx, starty;
    int endx, endy;
} MandelbrotThreadContext;

int mandelbrot_iterations_thread(void *ptr)
{
    MandelbrotThreadContext *c = ptr;
    mandelbrot_iterations_range(c->m, c->g, c->startx, c->starty, c->endx, c->endy);
    return 0;
}

void mandelbrot_iterations(Mandelbrot *m, Graph *g, int nthreads)
{
    if (nthreads <= 0) {
        mandelbrot_iterations_range(m, g, 0, 0, m->width, m->height);
        return;
    }

    SDL_Thread **threads = malloc(sizeof(SDL_Thread *) * nthreads);
    MandelbrotThreadContext *thread_contexts = malloc(sizeof(MandelbrotThreadContext) * nthreads);

    int rows_per_thread = m->height / nthreads;
    for (int i = 0; i < nthreads; i++) {
        // iterations data is structured like [row1][row2][row3][row4]....
        // so we will just split on the y-axis, any extra we will append to the last thread
        int starty = i * rows_per_thread;
        int endy = (i == nthreads - 1) ? m->height : starty + rows_per_thread;

        thread_contexts[i] =
            (MandelbrotThreadContext){.m = m, .g = g, .startx = 0, .endx = m->width, .starty = starty, .endy = endy};
        threads[i] = SDL_CreateThread(mandelbrot_iterations_thread, "MandelbrotIterationsThread", &thread_contexts[i]);
    }

    for (int i = 0; i < nthreads; i++) {
        SDL_WaitThread(threads[i], NULL);
    }

    free(threads);
    free(thread_contexts);
}

void render_mandelbrot(SDL_Renderer *renderer, SDL_Color *colors, int ncolors, Mandelbrot *m, Graph *g)
{
    int width = 0, height = 0;
    SDL_GetRendererOutputSize(renderer, &width, &height);

    int nthreads = 16;
    mandelbrot_iterations(m, g, nthreads);

    // Assign iterations to colors
    for (int py = 0; py < height; py++) {
        for (int px = 0; px < width; px++) {
            int iters = Mandelbrot_get_iterations(m, px, py);

            SDL_Color c = colors[iters % ncolors];
            SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, 0xff);
            SDL_RenderDrawPoint(renderer, px, py);
        }
    }
}

typedef struct RunContext {
    bool running;
    Graph *graph;
} RunContext;

bool process_events(RunContext *c)
{
    bool rerender = false;
    SDL_Event event;
    // process events
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                c->running = false;
                break;
            case SDL_KEYUP:
                switch (event.key.keysym.sym) {
                    case SDLK_q:
                        c->running = false;
                        break;
                    // scaling
                    case SDLK_EQUALS:
                        if (event.key.keysym.mod & KMOD_SHIFT) {
                            Graph_zoom(c->graph, 0.9f);
                            rerender = true;
                        }
                        break;
                    case SDLK_MINUS:
                        Graph_zoom(c->graph, 10.0f / 9.0f);
                        rerender = true;
                        break;
                    // moving
                    case SDLK_UP:
                        Graph_pan_up(c->graph, 10.0f);
                        rerender = true;
                        break;
                    case SDLK_DOWN:
                        Graph_pan_down(c->graph, 10.0f);
                        rerender = true;
                        break;
                    case SDLK_LEFT:
                        Graph_pan_left(c->graph, 10.0f);
                        rerender = true;
                        break;
                    case SDLK_RIGHT:
                        Graph_pan_right(c->graph, 10.0f);
                        rerender = true;
                        break;
                    // reset
                    case SDLK_r:
                        Graph_init_default(c->graph);
                        rerender = true;
                        break;
                    default:
                        break;
                }
                break;
            default:
                break;
        }
    }

    return rerender;
}

int main(void)
{
    int width = 700, height = 400;

    if (SDL_Init(SDL_INIT_EVERYTHING)) {
        fatal("Failed initializing SDL: %s\n", SDL_GetError());
    }

    SDL_Window *window = SDL_CreateWindow("Mandelbrot", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height,
                                          SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    if (!window) {
        fatal("Failed to create window: %s", SDL_GetError());
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        fatal("Failed to create renderer: %s", SDL_GetError());
    }

    SDL_Color colors[] = {{66, 30, 15, 255},    {25, 7, 26, 255},     {9, 1, 47, 255},      {4, 4, 73, 255},
                          {0, 7, 100, 255},     {12, 44, 138, 255},   {24, 82, 177, 255},   {57, 125, 209, 255},
                          {134, 181, 229, 255}, {211, 236, 248, 255}, {241, 233, 191, 255}, {248, 201, 95, 255},
                          {255, 170, 0, 255},   {204, 128, 0, 255},   {153, 87, 0, 255},    {106, 52, 3, 255}};

    // TODO: handle resize
    SDL_GetRendererOutputSize(renderer, &width, &height);  // runs at 2x on mac, need to get correct sizes
    Mandelbrot m;
    Mandelbrot_init_default(&m, width, height);

    Graph graph;
    Graph_init_default(&graph);

    RunContext ctx = {.running = true, .graph = &graph};
    uint32_t total_frames = 0;
    uint32_t total_frame_ticks = 0;
    while (ctx.running) {
        // Frame start
        total_frames += 1;
        uint32_t start_ticks = SDL_GetTicks();
        uint64_t start_perf = SDL_GetPerformanceCounter();

        // Clear the screen
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0xff);
        SDL_RenderClear(renderer);

        process_events(&ctx);

        // Render
        render_mandelbrot(renderer, colors, ARRAY_SIZE(colors), &m, &graph);

        // Frame end
        uint32_t end_ticks = SDL_GetTicks();
        uint64_t end_perf = SDL_GetPerformanceCounter();
        uint32_t frame_ticks = end_ticks - start_ticks;
        uint64_t frame_perf = end_perf - start_perf;

        total_frame_ticks += frame_ticks;
        float frame_time = (float)frame_ticks / 1000.0f;

        float fps = 1.0f / frame_time;
        float average_fps = 1000.0f / ((float)total_frame_ticks / (float)total_frames);

        // Render FPS
        printf("FPS: %.2f, AVG FPS: %.2f, Perf: %llu\r", fps, average_fps, frame_perf);

        // Show
        SDL_RenderPresent(renderer);
    }

    Mandelbrot_free(&m);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();
    return EXIT_SUCCESS;
}
