//
// Created by Zach Miller on 9/17/20.
//

#include "MandelbrotRenderer.h"

const Bounds2D<float>
    MandelbrotRenderer::DEFAULT_BOUNDS(Point2D<float>(-2.5, 1.0),
                                       Point2D<float>(-1.0, 1.0));


/*
void MandelbrotRenderer::resize(size_t new_width, size_t new_height) {
    auto xlen_perpix = man.bounds.xrange() / man.width();
    auto ylen_perpix = man.bounds.yrange() / man.height();

    auto scaled_half = Point2D<float>(new_width * xlen_perpix / 2.0,
                                      new_height * ylen_perpix / 2.0);
    auto bl = man.bounds.center() - scaled_half;
    auto tr = man.bounds.center() + scaled_half;

    Bounds2D<float_type> new_bounds(bl, tr);
    setBounds(new_bounds);
}
 */


void MandelbrotRenderer::setBounds(const Bounds2D<float_type> &newBounds) {
    man = Mandelbrot<float_type>(man.width(), man.height(), man.max_iterations,
                                 newBounds);
    updatePixels();
}

void MandelbrotRenderer::shiftHorizontal(int ticks) {
    setBounds(Bounds2D<float_type>(
        man.bounds.bl + Point2D<float_type>(step * ticks, 0),
        man.bounds.tr + Point2D<float_type>(step * ticks, 0)));
}

void MandelbrotRenderer::shiftVertical(int ticks) {
    setBounds(Bounds2D<float_type>(
        man.bounds.bl + Point2D<float_type>(0, step * ticks),
        man.bounds.tr + Point2D<float_type>(0, step * ticks)));
}

void MandelbrotRenderer::zoom(float amt) {
    auto xamt = man.bounds.xrange() * amt / 2.0;
    auto yamt = man.bounds.yrange() * amt / 2.0;
    Point2D<float_type> shift(xamt, yamt);

    auto blmul = amt < 1.0 ? 1.0f : -1.0f;
    auto trmul = amt < 1.0 ? -1.0f : 1.0f;
    Bounds2D<float_type> bounds(man.bounds.bl + shift * blmul,
                                man.bounds.tr + shift * trmul);

    step *= amt;
    setBounds(bounds);
}

void MandelbrotRenderer::defaultPosition() {
    step = 0.25;
    setBounds(DEFAULT_BOUNDS);
}

void MandelbrotRenderer::relativeShift(int x0, int y0, int x1, int y1) {
    auto xamt = (man.bounds.xrange() * (x1 - x0)) / man.width();
    auto yamt = (man.bounds.yrange() * (y1 - y0)) / man.height();

    Point2D<float_type> shift(-xamt, -yamt);

    Bounds2D<float_type> bounds(man.bounds.bl + shift, man.bounds.tr + shift);

    setBounds(bounds);
}
void MandelbrotRenderer::updatePixels() {
    for (size_t row = 0; row < man.height(); row++) {
        for (size_t col = 0; col < man.width(); col++) {
            auto iters = man.iterations[row][col];
            auto color = palette[iters % palette.size()];
            pixels[row][col] = color;
        }
    }
}
void MandelbrotRenderer::onResize(const WindowGrid<int, float> &grid) {
    // TODO
}
