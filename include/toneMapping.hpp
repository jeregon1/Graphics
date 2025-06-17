#pragma once

#include "Image.hpp"
#include <functional>

constexpr float DEFAULT_TONE_MAPPING_GAMMA = 2.2f;
constexpr float DEFAULT_TONE_MAPPING_MAX = 1.0f;
constexpr float DEFAULT_TONE_MAPPING_KEY = 0.18f;
constexpr float DEFAULT_TONE_MAPPING_LWHITE = 1.0f;

// Forward declaration
enum class ToneMappingType;
struct RenderConfig;

// Modern functional approach to tone mapping
namespace ToneMapping {
    void clamp(Image& image, float max = DEFAULT_TONE_MAPPING_MAX) noexcept;
    void equalization(Image& image, float V = 0.0f) noexcept;
    void equalizationClamp(Image& image, float max = DEFAULT_TONE_MAPPING_MAX) noexcept;
    void gamma(Image& image, float gammaValue = DEFAULT_TONE_MAPPING_GAMMA) noexcept;
    void clampGamma(Image& image, float max = DEFAULT_TONE_MAPPING_MAX, float gammaValue = DEFAULT_TONE_MAPPING_GAMMA) noexcept;
    void reinhard(Image& img, float key = DEFAULT_TONE_MAPPING_KEY, float Lwhite = DEFAULT_TONE_MAPPING_LWHITE) noexcept;
    // Apply tone mapping based on config
    void apply(Image& image, const RenderConfig& config) noexcept;
    
    // Functional version that returns new image instead of modifying
    [[nodiscard]] Image apply(const Image& img, std::function<RGB(const RGB&)> transform) noexcept;
}
