/*
 * Image.hpp
 * Date: 2024/09/30
 * Author: Jesús López Ansón, 839922
 * Author: Pablo terés Pueyo, 843350
 * Description: Definition of Image class
 */

#pragma once

#include <string>
#include <vector>
#include <optional>
#include <cmath>
#include <algorithm>

#include "RGB.hpp"

class Image {
public:
   int width{0}, height{0};
   std::vector<RGB> pixels;

   Image() = default;
   constexpr Image(int width, int height) noexcept 
         : width(width), height(height) {}

   Image(int width, int height, std::vector<RGB> pixels) noexcept
         : width(width), height(height), pixels(std::move(pixels)) {}

   explicit Image(const std::string& path); // explicit prevents implicit conversions

   // Move constructor and assignment for better performance
   Image(Image&&) noexcept = default;
   Image& operator=(Image&&) noexcept = default;
   
   // Copy operations
   Image(const Image&) = default;
   Image& operator=(const Image&) = default;

   [[nodiscard]] float max() const noexcept;

   [[nodiscard]] static std::optional<Image> readPPM(const std::string& path);
   bool writePPM(const std::string& path) const noexcept;

   [[nodiscard]] static std::optional<Image> readBMP(const std::string& path);
   bool writeBMP(const std::string& path) const noexcept;
   
   // Utility functions
   [[nodiscard]] bool empty() const noexcept { return pixels.empty(); }
   [[nodiscard]] size_t size() const noexcept { return pixels.size(); }
};
