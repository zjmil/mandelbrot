
#include <array>
#include <numeric>
#include <stdexcept>

#include <SDL.h>
#include <SDL_ttf.h>

#include "GameConfig.h"
#include "Grid.h"
#include "Mandelbrot.h"
#include "MandelbrotRenderer.h"
#include "WindowGrid.h"

void fatal(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, fmt, args);

    va_end(args);

    exit(EXIT_FAILURE);
}

class FPS {
  private:
    uint32_t nframes = 0;
    std::array<uint32_t, 16> elapsed_times = {0};

    uint32_t avg_elapsed_times() const {
        auto total =
            std::accumulate(elapsed_times.begin(), elapsed_times.end(), 0UL);
        return total / elapsed_times.size();
    };

  public:
    FPS() = default;

    void record_elapsed(uint32_t millis) {
        elapsed_times[nframes++ % elapsed_times.size()] = millis;
    };
    float fps() const { return 1000.0f / avg_elapsed_times(); };
};

static const SDL_Color white = {255, 255, 255, 255};
static const SDL_Color black = {0, 0, 0, 255};

class FPSOverlay {
  public:
    FPSOverlay(FPS *fps, TTF_Font *font) : fps(fps), font(font){};

    void render(SDL_Renderer *renderer) const {
        std::string fps_text(8, '\0');
        std::snprintf(&fps_text[0], fps_text.size(), "%.2f", fps->fps());

        SDL_Surface *text_surface = TTF_RenderText_Solid(font, fps_text.c_str(), white);
        SDL_Texture *text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
        SDL_Rect text_rect = {.x = 0, .y = 0, .w = 60, .h = 40};

        SDL_RenderCopy(renderer, text_texture, nullptr, &text_rect);

        SDL_DestroyTexture(text_texture);
        SDL_FreeSurface(text_surface);
    }

  private:
    FPS *fps;
    TTF_Font *font;
};

int main(int argc, char *argv[]) {

    (void)argc;

    WindowGrid<int, float> windowGrid(700, 400, -2.5f, -1.0f, 1.0f, 1.0f);

    if (SDL_Init(SDL_INIT_EVERYTHING)) {
        fatal("Failed initializing SDL: %s\n", SDL_GetError());
    }

    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "%s: %d.%d", argv[0],
                Game_VERSION_MAJOR, Game_VERSION_MINOR);

    SDL_Window *window = SDL_CreateWindow(Game_NAME, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, windowGrid.width(), windowGrid.height(), SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE); // NOLINT(hicpp-signed-bitwise)
    if (!window) {
        fatal("Failed to create window: %s", SDL_GetError());
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        fatal("Failed to create renderer: %s", SDL_GetError());
    }

    SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, windowGrid.width(), windowGrid.height());
    if (!texture) {
        fatal("Failed to create texture: %s", SDL_GetError());
    }

    std::vector<Pixel> colors = {{{66, 30, 15, 255},
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
                                  {106, 52, 3, 255}}};

    MandelbrotRenderer man(windowGrid.width(), windowGrid.height(), 100, colors);

    TTF_Init();
    const auto font_name = "/System/Library/Fonts/SFNSMono.ttf";
    const auto font_size = 24;
    TTF_Font *font = TTF_OpenFont(font_name, font_size);
    if (!font) {
        fatal("Failed to load font: %s", TTF_GetError()); // TODO: error message?
    }

    FPS fps;
    FPSOverlay fps_overlay(&fps, font);

    GridRenderer grid;

    int mousedownx = 0, mousedowny = 0;
    SDL_Event event;
    auto running = true;
    auto paused = false;
    auto last_end_time = SDL_GetTicks();
    while (running) {
        // clear the screen
        SDL_SetRenderDrawColor(renderer, black.r, black.g, black.b, black.a);
        SDL_RenderClear(renderer);

        // process events
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT:
                running = false;
                break;
            case SDL_KEYUP:
                switch (event.key.keysym.sym) {
                case SDLK_UP:
                    man.shiftVertical(-1);
                    break;
                case SDLK_DOWN:
                    man.shiftVertical(1);
                    break;
                case SDLK_LEFT:
                    man.shiftHorizontal(-1);
                    break;
                case SDLK_RIGHT:
                    man.shiftHorizontal(1);
                    break;
                case SDLK_EQUALS:
                    if (event.key.keysym.mod & KMOD_SHIFT) { // NOLINT(hicpp-signed-bitwise)
                        man.zoom(0.75);
                    }
                    break;
                case SDLK_MINUS:
                    man.zoom(1.0 / 0.75);
                    break;
                case SDLK_r:
                    man.defaultPosition();
                    break;
                case SDLK_q:
                    running = false;
                    break;
                }
                break;
            case SDL_WINDOWEVENT:
                switch (event.window.event) {
                case SDL_WINDOWEVENT_SIZE_CHANGED:
                    windowGrid.resize(event.window.data1, event.window.data2);
                    break;
                }
            case SDL_MOUSEBUTTONDOWN:
                if (event.button.button == SDL_BUTTON_LEFT) {
                    mousedownx = event.button.x;
                    mousedowny = event.button.y;
                }
                break;
            case SDL_MOUSEBUTTONUP:
                if (event.button.button == SDL_BUTTON_LEFT) {
                    auto mouseupx = event.button.x;
                    auto mouseupy = event.button.y;
                    man.relativeShift(mousedownx, mousedowny, mouseupx,
                                      mouseupy);
                }
                break;
            case SDLK_ESCAPE:
                paused = !paused;
                break;
            default:
                break;
            }
        }

        if (paused) {
        } else {
            // draw to texture
            // TODO: don't need to copy if not re-rendered
            SDL_UpdateTexture(texture, nullptr, man.pixelData(), windowGrid.width() * 4);
            SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        }

        grid.render(renderer, windowGrid.width(), windowGrid.height());

        // render FPS
        fps_overlay.render(renderer);

        // present rendered
        SDL_RenderPresent(renderer);

        // update fps
        auto curr_time = SDL_GetTicks();
        auto elapsed_time = curr_time - last_end_time;
        last_end_time = curr_time;
        fps.record_elapsed(elapsed_time);
    }

    TTF_CloseFont(font);
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();
    return EXIT_SUCCESS;
}
