/**
    File: RGB.hpp
    Date: 2024/09/30
    Authors: Jesús López Ansón, 839922
             Pablo terés Pueyo, 843350
    Description: Implementation of RGB struct
 */

#pragma once

#include <iostream>
#include <vector>
#include <cmath>

struct RGB {
    int r;
    int g;
    int b;

    RGB() : r(0), g(0), b(0) {};
    RGB(int r, int g, int b) : r(r), g(g), b(b) {};
    RGB(const RGB &rgb) : r(rgb.r), g(rgb.g), b(rgb.b) {};

    RGB operator+(const RGB &rgb) const;
    RGB operator-(const RGB &rgb) const;
    RGB operator*(const RGB &rgb) const;
    RGB operator/(const RGB &rgb) const;

    RGB operator+(int value) const;
    RGB operator-(int value) const;
    RGB operator*(int value) const;
    RGB operator/(int value) const;

    RGB operator+=(const RGB &rgb);
    RGB operator-=(const RGB &rgb);
    RGB operator*=(const RGB &rgb);
    RGB operator/=(const RGB &rgb);

    RGB operator+=(int value);
    RGB operator-=(int value);
    RGB operator*=(int value);
    RGB operator/=(int value);

    bool operator==(const RGB &rgb) const;
    bool operator!=(const RGB &rgb) const;

    RGB operator=(const RGB &rgb);

    friend std::ostream &operator<<(std::ostream &os, const RGB &rgb);
    friend std::istream &operator>>(std::istream &is, RGB &rgb);

    void clamp(int min, int max);
    void equalization(const std::vector<int> &histogram);
    void gamma(float gamma);
};
