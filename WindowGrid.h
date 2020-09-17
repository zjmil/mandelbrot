#ifndef WINDOW_GRID_H
#define WINDOW_GRID_H


template <typename S, typename T> class WindowGrid {
  private:
    S _width, _height;
    T _xmin, _ymin, _xmax, _ymax;

  public:
    WindowGrid(S width, S height, T xmin, T ymin, T xmax, T ymax)
        : _width(width), _height(height), _xmin(xmin), _ymin(ymin), _xmax(xmax),
          _ymax(ymax){};

    constexpr S width() const { return _width; }
    constexpr S height() const { return _height; }

    constexpr T xmin() const { return _xmin; }
    constexpr T xmax() const { return _xmax; }
    constexpr T ymin() const { return _ymin; }
    constexpr T ymax() const { return _ymax; }

    constexpr T xrange() const { return _xmax - _xmin; }
    constexpr T yrange() const { return _ymax - _ymin; }

    constexpr T xdelta() const { return _width / xrange(); }
    constexpr T ydelta() const { return _height / yrange(); }

    constexpr T xcenter() const { return _xmin + xrange() / 2.0; }
    constexpr T ycenter() const { return _ymin + yrange() / 2.0; }

    void resize(S new_width, S new_height) {
        auto new_xrange = xrange() * new_width / _width;
        auto new_xmin = xcenter() - new_xrange / 2.0;
        auto new_xmax = xcenter() + new_xrange / 2.0;

        auto new_yrange = yrange() * new_height / _height;
        auto new_ymin = ycenter() - new_yrange / 2.0;
        auto new_ymax = ycenter() + new_yrange / 2.0;

        _xmin = new_xmin;
        _xmax = new_xmax;
        _ymin = new_ymin;
        _ymax = new_ymax;
        _width = new_width;
        _height = new_height;
    }
};

#endif /* WINDOW_GRID_H */
