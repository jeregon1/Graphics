/*
 * Image.hpp
 * Date: 2024/09/30
 * Author: Jesús López Ansón, 839922
 * Author: Pablo terés Pueyo, 843350
 * Description: Definition of Image class
 */

#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <cmath>
#include <algorithm>

#include "RGB.hpp"

using namespace std;


struct Image {

   static constexpr float MEMORY_COLOR_RESOLUTION = 18.35;
   static constexpr float DISK_COLOR_RESOLUTION = 2e30;

   int width, height;
   vector<RGB> pixels;
   float memoryColorResolution;

   Image() {}

   Image(int width, int height, float memoryColorRes = MEMORY_COLOR_RESOLUTION) 
         : width(width), height(height), memoryColorResolution(memoryColorRes) {}

   Image(int width, int height, vector<RGB> pixels, float memoryColorRes = MEMORY_COLOR_RESOLUTION) 
         : width(width), height(height), pixels(pixels), memoryColorResolution(memoryColorRes) {}

   Image(const string& path);

   float max() const;

   static Image readPPM(const string& path);
   void writePPM(const string& path, float diskColorRes = DISK_COLOR_RESOLUTION) const;

   static Image readBMP(const string& path);
   void writeBMP(const string& path) const;
};
