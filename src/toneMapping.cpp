/*
 * toneMapping.cpp
 * Date: 2024/09/30
 * Author: Jesús López Ansón, 839922
 * Author: Pablo terés Pueyo, 843350
 * Description: Implementation of ToneMapping operators
    - Clamp
    - Equalization
    - Equalization + Clamp
    - Gamma
    - Clamp + Gamma
 */

#include "../include/toneMapping.hpp"
#include <algorithm>
#include <vector>
#include <cmath>
#include <functional>

namespace ToneMapping {

// Each RGB value is clamped to the range [0, max]
void clamp(Image& image, float max) noexcept {
   for (auto& pixel : image.pixels) {
      pixel.r = std::clamp(pixel.r, 0.0f, max);
      pixel.g = std::clamp(pixel.g, 0.0f, max);
      pixel.b = std::clamp(pixel.b, 0.0f, max);
   }
}

// Functional approach that returns new image
Image apply(const Image& img, std::function<RGB(const RGB&)> transform) noexcept {
    Image result = img; // Copy
    std::transform(img.pixels.begin(), img.pixels.end(), 
                   result.pixels.begin(), transform);
    return result;
}

// Normalization of all the values in a linear way
void equalization(Image& image, float V) noexcept {
   if (V == 0) {
      V = std::max_element(image.pixels.begin(), image.pixels.end(), [](const RGB& a, const RGB& b) {
         return a.max() < b.max();
      })->max();
   }

   for (auto& pixel : image.pixels) {
      pixel /= V;
   }
}

void equalizationClamp(Image& image, float V) noexcept {
   equalization(image, V);
   clamp(image, V);
}

void gamma(Image& image, float gammaValue) noexcept {
   equalization(image, 0);
   for (auto& pixel : image.pixels) {
      pixel = pixel.pow(1 / gammaValue);
   }
}

void clampGamma(Image& image, float max, float gammaValue) noexcept {
   equalizationClamp(image, max);
   gamma(image, gammaValue);
}

void reinhard(Image& img, float key, float Lwhite) noexcept {
   std::vector<RGB> output(img.pixels.size());
    
   // Step 1: Calculate the log average luminance
   float sumLuminance = 0.0f;
   float maxLuminance = 0.0f;
   std::vector<float> luminances(img.pixels.size());
   
   for (size_t i = 0; i < img.pixels.size(); ++i) {
      // Convert RGB to luminance using the perceptual weights
      float luminance = 0.2126f * img.pixels[i].r + 
                        0.7152f * img.pixels[i].g + 
                        0.0722f * img.pixels[i].b;
      
      luminances[i] = std::max(luminance, 0.0001f); // Avoid taking log of zero
      maxLuminance = std::max(maxLuminance, luminance);
      sumLuminance += std::log(luminance);
   }
   
   float logAvgLuminance = std::exp(sumLuminance / img.pixels.size());
   
   // Step 2: Scale the luminance by the key value
   float scaledLuminance = key / logAvgLuminance;
   
   // If Lwhite is not provided, use the maximum luminance
   if (Lwhite <= 0.0f) {
      Lwhite = maxLuminance;
   }
   
   float Lwhite2 = Lwhite * Lwhite;
   
   // Step 3: Apply tone mapping to each pixel
   for (size_t i = 0; i < img.pixels.size(); ++i) {
      float Lw = luminances[i];
      float Lw_scaled = Lw * scaledLuminance;
      
      // Extended Reinhard formula with Lwhite parameter
      float numerator = Lw_scaled * (1.0f + (Lw_scaled / Lwhite2));
      float denominator = 1.0f + Lw_scaled;
      float Ld = numerator / denominator;
      
      // Calculate the scaling factor to preserve colors
      float scale = Ld / Lw;
      
      // Apply the scaling to RGB channels
      output[i] = img.pixels[i] * scale;
   }
   
   img.pixels = std::move(output);
}

} // namespace ToneMapping

