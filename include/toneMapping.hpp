/*
 * toneMapping.hpp
 * Date: 2024/09/30
 * Author: Jesús López Ansón, 839922
 * Author: Pablo terés Pueyo, 843350
 * Description: Definition of ToneMapping operators
    - Clamp
    - Equalization
    - Clamp + Equalization
    - Gamma
    - Clamp + Gamma
 */

#pragma once

#include "Image.hpp"

#define GAMMA 2.2

void clamp(Image& image, float max = 1);
void equalization(Image& image, float V = 0);
void equalizationClamp(Image& image, float max = 1);
void gamma(Image& image, float gammaValue = GAMMA);
void clampGamma(Image& image, float max = 1, float gammaValue = GAMMA);
void reinhard(Image& img, float key = 0.18, float Lwhite = 1);
