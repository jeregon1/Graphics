/*
 * ppmReaderWriter.hpp
 * Date: 2024/09/30
 * Author: Jesús López Ansón, 839922
 * Author: Pablo terés Pueyo, 843350
 * Description: Definición de operadores de ToneMapping
    - Clamp
    - Equalization
    - Clamp + Equalization
    - Gamma
    - Clamp + Gamma
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

#define MAX 18.35

struct Image {

   int width, height;
   vector<RGB> pixels;

   Image() {}

   Image(int width, int height) : width(width), height(height) {}

   Image(int width, int height, vector<RGB>& pixels) : width(width), height(height), pixels(pixels) {}

   Image(const Image &image) : width(image.width), height(image.height), pixels(image.pixels) {}

   Image(string PPMfilename) {
      *this = readPPM(PPMfilename);
   }

   static Image readPPM(string filename);

   void writePPM(string filename, float maxColor);
};