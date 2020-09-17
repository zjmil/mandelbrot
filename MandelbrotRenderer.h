//
// Created by Zach Miller on 9/17/20.
//

#ifndef GAME_MANDELBROTRENDERER_H
#define GAME_MANDELBROTRENDERER_H

#include <vector>

#include "Mandelbrot.h"

struct Pixel {
    uint8_t r, g, b, a;
};

class MandelbrotRenderer {
  private:
    using float_type = float;

    static const Bounds2D<float_type> DEFAULT_BOUNDS;

    Matrix2D<Pixel> pixels;
    std::vector<Pixel> palette;

    Mandelbrot<float_type> man;
    float_type step = 0.25;

    void updatePixels();

    void setBounds(const Bounds2D<float_type> &newBounds);

  public:
    MandelbrotRenderer(size_t width, size_t height, int max_iterations,
                       std::__1::vector<Pixel> palette)
        : pixels(width, height), palette(std::move(palette)),
          man(width, height, max_iterations,
                                     DEFAULT_BOUNDS) {
        updatePixels();
    };

    void onResize(const WindowGrid<int, float>& grid);
    void shiftHorizontal(int ticks);
    void shiftVertical(int ticks);
    void zoom(float amt);
    void defaultPosition();
    void relativeShift(int x0, int y0, int x1, int y1);

    const void *pixelData() const { return pixels.data(); };
};

#endif // GAME_MANDELBROTRENDERER_H
