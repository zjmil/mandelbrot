#include "Grid.h"

void GridRenderer::render(SDL_Renderer *renderer, int width, int height) const {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

    // horizontal
    SDL_RenderDrawLine(renderer, 0, height / 2, width, height / 2);
    // vertical
    SDL_RenderDrawLine(renderer, width / 2, 0, width / 2, height);

    // TODO: need bounds
    if (with_dashes) {
        // horizontal
        int hndashes = 8; // TODO
        int hdashlen = 0.05 * height;
        int hdashspacing = width / hndashes;
        for (int i = 0; i < hndashes; i++) {
            int x1 = i * hdashspacing;
            int y1 = height / 2 - hdashlen / 2;
            int x2 = x1;
            int y2 = y1 + hdashlen;

            SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
        }

        // vertical
        int vndashes = 8; // TODO
        int vdashlen = 0.05 * width;
        int vdashspacing = height / vndashes;
        for (int i = 0; i < vndashes; i++) {
            int x1 = width / 2 - vdashlen / 2;
            int y1 = i * vdashspacing;
            int x2 = x1 + vdashlen;
            int y2 = y1;

            SDL_RenderDrawLine(renderer, x1, y1, x2, y2);
        }
    }
}
