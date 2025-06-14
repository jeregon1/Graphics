/**
    File: RGB.hpp
    Date: 2024/09/30
    Authors: Jesús López Ansón, 839922
            Pablo terés Pueyo, 843350
    Description: Definition and implementation of RGB struct
 */

#pragma once

#include <iostream>
#include <algorithm>
#include <cmath>

struct RGB {
    float r, g, b;

    static constexpr float tolerance = 1e-6f;

    constexpr RGB() noexcept : r(0), g(0), b(0) {}
    constexpr RGB(float r, float g, float b) noexcept : r(r), g(g), b(b) {}
    constexpr RGB(const RGB&) noexcept = default;
    constexpr RGB& operator=(const RGB&) noexcept = default;

    constexpr RGB operator+(const RGB &rgb) const noexcept {
        return RGB(r + rgb.r, g + rgb.g, b + rgb.b);
    }

    constexpr RGB operator-(const RGB &rgb) const noexcept {
        return RGB(r - rgb.r, g - rgb.g, b - rgb.b);
    }

    constexpr RGB operator*(const RGB &rgb) const noexcept {
        return RGB(r * rgb.r, g * rgb.g, b * rgb.b);
    }

    constexpr RGB operator/(const RGB &rgb) const noexcept {
        return RGB(r / rgb.r, g / rgb.g, b / rgb.b);
    }

    template <typename T>
    RGB operator+(T value) const {
        return RGB(r + value, g + value, b + value);
    }

    RGB operator-(float value) const {
        return RGB(r - value, g - value, b - value);
    }

    RGB operator*(float value) const {
        return RGB(r * value, g * value, b * value);
    }

    RGB operator/(float value) const {
        return RGB(r / value, g / value, b / value);
    }

    RGB pow(float value) const {
        return RGB(std::pow(r, value), std::pow(g, value), std::pow(b, value));
    }

    float max() const {
        return std::max(std::max(r, g), b);
    }

    float min() const {
        return std::min(r, std::min(g, b));
    }

    RGB clamp(float minVal = 0.f, float maxVal = 1.f) const {
        return RGB(
            std::clamp(r, minVal, maxVal),
            std::clamp(g, minVal, maxVal),
            std::clamp(b, minVal, maxVal)
        );
    }

    RGB& operator+=(const RGB &rgb) {
        return *this = *this + rgb;
    }

    RGB& operator-=(const RGB &rgb) {
        return *this = *this - rgb;
    }

    RGB& operator*=(const RGB &rgb) {
        return *this = *this * rgb;
    }

    RGB& operator/=(const RGB &rgb) {
        return *this = *this / rgb;
    }

    RGB& operator+=(float value) {
        return *this = *this + value;
    }

    RGB& operator-=(float value) {
        return *this = *this - value;
    }

    RGB& operator*=(float value) {
        return *this = *this * value;
    }

    RGB& operator/=(float value) {
        return *this = *this / value;
    }

    bool operator==(const RGB &rgb) const {
        return abs(r - rgb.r) < tolerance 
            && abs(g - rgb.g) < tolerance 
            && abs(b - rgb.b) < tolerance;
    }

    bool operator!=(const RGB &rgb) const {
        return !(*this == rgb);
    }

    friend std::ostream &operator<<(std::ostream &os, const RGB &rgb) {
        os << rgb.r << " " << rgb.g << " " << rgb.b;
        return os;
    }

    friend std::istream &operator>>(std::istream &is, RGB &rgb) {
        is >> rgb.r >> rgb.g >> rgb.b;
        return is;
    }
};

inline RGB round(RGB pixel) {
    return RGB(std::round(pixel.r), std::round(pixel.g), std::round(pixel.b));
}
