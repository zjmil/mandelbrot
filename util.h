#ifndef UTIL_H
#define UTIL_H

#include <cmath>

template <typename T> constexpr auto approx(T a, T b, T eps) {
    return abs(a - b) < eps;
}

#endif /* UTIL_H */
