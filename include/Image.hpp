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

   int width, height;
   vector<RGB> pixels;

   Image() {}

   Image(int width, int height) 
         : width(width), height(height) {}

   Image(int width, int height, vector<RGB> pixels) 
         : width(width), height(height), pixels(pixels) {}

   Image(const string& path);

   float max() const;

   static Image readPPM(const string& path);
   void writePPM(const string& path) const;

   static Image readBMP(const string& path);
   void writeBMP(const string& path) const;
};
