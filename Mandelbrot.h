#ifndef MANDELBROT_H
#define MANDELBROT_H

#include <array>
#include <thread>
#include <utility>
#include <vector>

#include <SDL.h>

#include "WindowGrid.h"
#include "util.h"

template <typename T> class Matrix2D;

template <typename T> class RowView {
  public:
    T &operator[](size_t col);

    friend class Matrix2D<T>; // for private constructor
  private:
    class Matrix2D<T> *m;
    size_t row;

    RowView(Matrix2D<T> *m, size_t row) : m(m), row(row){};
};

template <typename T> class Matrix2D {
  private:
    std::vector<T> elements;

    friend class RowView<T>; // for elements access
  public:
    size_t width, height;

    Matrix2D() : width(0), height(0){};
    Matrix2D(size_t width, size_t height)
        : elements(std::vector<T>(width * height)), width(width),
          height(height){};

    size_t size() const { return elements.size(); }
    const T *data() const { return elements.data(); }

    RowView<T> operator[](size_t row) { return RowView<T>(this, row); }
};

template <typename T> T &RowView<T>::operator[](size_t col) {
    return m->elements[m->width * row + col];
}

template <typename T> struct Point2D {
    T x, y;

    Point2D(const T &x, const T &y) : x(x), y(y){};

    Point2D<T> operator+(const Point2D<T> &other) {
        return Point2D<T>(x + other.x, y + other.y);
    }

    Point2D<T> operator-(const Point2D<T> &other) {
        return Point2D<T>(x - other.x, y - other.y);
    }

    Point2D<T> operator*(const T &amt) { return Point2D<T>(amt * x, amt * y); }

    Point2D<T> operator/(const T &amt) { return Point2D<T>(x / amt, y / amt); }
};

template <typename T> struct Bounds2D {
    using P = Point2D<T>;
    P bl, tr;

    Bounds2D() : bl(P(0, 0)), tr(P(0, 0)){};
    Bounds2D(const T &blx, const T &bly, const T &trx, const T &try_)
        : bl(P(blx, bly)), tr(P(trx, try_)){};
    Bounds2D(const P &bl, const P &tr) : bl(bl), tr(tr){};

    constexpr T xrange() const { return tr.x - bl.x; }
    constexpr T yrange() const { return tr.y - bl.y; }
    constexpr T xcenter() const { return bl.x + xrange() / 2.0; }
    constexpr T ycenter() const { return bl.y + yrange() / 2.0; }

    constexpr Point2D<T> center() const {
        return Point2D<T>(xcenter(), ycenter());
    }
};

template <typename F> class Mandelbrot {
  public:
    int max_iterations;
    Matrix2D<int> iterations;
    Bounds2D<F> bounds;

    Mandelbrot(): max_iterations(0), iterations(0, 0), bounds(0, 0, 0, 0) {}
    Mandelbrot(size_t width, size_t height, int max_iterations,
               Bounds2D<F> bounds)
        : max_iterations(max_iterations),
          iterations(width, height), bounds(bounds) {}

    constexpr size_t height() const { return iterations.height; }
    constexpr size_t width() const { return iterations.width; }

  protected:
    int cutoff_period = 20;

    constexpr auto xdelta() const noexcept { return bounds.xrange() / width(); }
    constexpr auto ydelta() const noexcept {
        return bounds.yrange() / height();
    }

    int calc_iterations(const F x0, const F y0) const {
        auto x = 0.0, y = 0.0, x2 = 0.0, y2 = 0.0, xold = 0.0, yold = 0.0;
        auto iters = 0, period = 0;
        const auto eps = 1.0e-7;
        while (x2 + y2 <= 4.0 && iters < max_iterations) {
            y = 2 * x * y + y0;
            x = x2 - y2 + x0;

            x2 = x * x;
            y2 = y * y;

            // hasn't really moved, jump to the end
            if (approx(x, xold, eps) && approx(y, yold, eps)) {
                iters = max_iterations;
                break;
            }

            period += 1;
            if (period > cutoff_period) {
                period = 0;
                xold = x;
                yold = y;
            }

            iters += 1;
        }

        return iters;
    }

    void run_range(size_t pystart, size_t pyend) {
        auto xd = xdelta(), yd = ydelta();

        auto y = bounds.bl.y + yd * pystart;
        for (size_t py = pystart; py < pyend; ++py) {
            auto x = bounds.bl.x;
            for (size_t px = 0; px < width(); ++px) {
                iterations[py][px] = calc_iterations(x, y);
                x += xd;
            }
            y += yd;
        }
    }

    virtual void run() { run_range(0, height()); }
};

template <typename F> class ThreadedMandelbrot : public Mandelbrot<F> {
  public:
    ThreadedMandelbrot(size_t width, size_t height, int max_iterations,
                       Bounds2D<F> bounds, int nthreads)
        : Mandelbrot<F>(width, height, max_iterations, bounds),
          nthreads(nthreads) {}

  private:
    int nthreads;

    virtual void run() {
        std::vector<std::thread> threads;

        size_t pystart = 0, pyend = 0;
        size_t step = this->height() / nthreads;
        for (auto i = 0; i < nthreads; i++) {
            if (i == nthreads - 1) {
                pyend = this->height();
            } else {
                pyend = pystart + step;
            }
            threads.push_back(std::thread(&ThreadedMandelbrot::run_range, this,
                                          pystart, pyend));
            pystart = pyend;
        }

        for (auto &thread : threads) {
            thread.join();
        }
    }
};

#endif /* MANDELBROT_H */
