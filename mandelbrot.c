
#include <stdlib.h>
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

bool approxf(float a, float b)
{
    return fabsf(a - b) < 1.0e-7;
}

typedef struct Point2D {
    float x, y;
} Point2D;

typedef struct Graph {
    Point2D center;
    float scale; // TODO: add different xscale and yscale for resize events
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
} Mandelbrot;

void Mandelbrot_init_default(Mandelbrot *m)
{
    m->max_iterations = 1000;
    m->max_periods = 20;
}

int mandelbrot_iterations(Point2D p0, int max_iterations, int max_periods)
{
    float x = 0.0f, y = 0.0f, x2 = 0.0f, y2 = 0.0f;
    int iterations = 0;

    Point2D old = {.x = 0.0f, .y = 0.0f};
    int period = 0;

    while ((x2 + y2) <= 4.0 && iterations < max_iterations) {
        y = 2*x*y + p0.y;
        x = x2 - y2 + p0.x;
        x2 = x*x;
        y2 = y*y;

        iterations += 1;

        if (approxf(x, old.x) && approxf(y, old.y)) {
            iterations = max_iterations;
            break;
        }

        period += 1;
        if (period > max_periods) {
            period = 0;
            old = (Point2D){.x = x, .y = y};
        }
    }

    return iterations;
}

void render_mandelbrot(SDL_Renderer *renderer, SDL_Color *colors, int ncolors, Mandelbrot *m, Graph *g)
{
    int width = 0, height = 0;
    SDL_GetRendererOutputSize(renderer, &width, &height);

    float tx = g->center.x - (g->scale * width) / 2.0f;
    float ty = g->center.y + (g->scale * height) / 2.0f;

    Point2D p0;
    for (int py = 0; py < height; py++) {
        p0.y = ty - g->scale * py;

        for (int px = 0; px < width; px++) {
            p0.x = tx + g->scale * px;

            int iterations = mandelbrot_iterations(p0, m->max_iterations, m->max_periods);

            SDL_Color c = colors[iterations % ncolors];
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

    SDL_Window *window = SDL_CreateWindow(
            "Mandelbrot",
            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
            width, height,
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

    Mandelbrot m;
    Mandelbrot_init_default(&m);

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

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();
    return EXIT_SUCCESS;
}
