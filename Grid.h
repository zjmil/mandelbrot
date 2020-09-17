#ifndef GRID_H
#define GRID_H

#include <SDL.h>

class GridRenderer {
  public:
    GridRenderer() = default;;

    void render(SDL_Renderer *renderer, int width, int height) const;

  private:
    SDL_Color color = {255, 255, 255, 255};
    bool with_dashes = true;
};

#endif /* GRID_H */
