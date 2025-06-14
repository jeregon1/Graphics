#pragma once

#include "Image.hpp"
#include <functional>

constexpr float DEFAULT_GAMMA = 2.2f;

// Modern functional approach to tone mapping
namespace ToneMapping {
    void clamp(Image& image, float max = 1.0f) noexcept;
    void equalization(Image& image, float V = 0.0f) noexcept;
    void equalizationClamp(Image& image, float max = 1.0f) noexcept;
    void gamma(Image& image, float gammaValue = DEFAULT_GAMMA) noexcept;
    void clampGamma(Image& image, float max = 1.0f, float gammaValue = DEFAULT_GAMMA) noexcept;
    void reinhard(Image& img, float key = 0.18f, float Lwhite = 1.0f) noexcept;
    
    // Functional version that returns new image instead of modifying
    [[nodiscard]] Image apply(const Image& img, std::function<RGB(const RGB&)> transform) noexcept;
}
