#pragma once

#include <random>
#include <cmath>
#include <cstdlib>
#include "geometry.hpp"

//https:projecteuclid.org/journals/annals-of-mathematical-statistics/volume-43/issue-2/Choosing-a-Point-from-the-Surface-of-a-Sphere/10.1214/aoms/1177692644.full

namespace {
    static std::mt19937 rng(std::random_device{}());
    static std::uniform_real_distribution<double> dist01(0.0, 1.0);
}

inline double rand0_1() {
    return dist01(rng);
}

/* 
 *  Este código implementa una función para muestrear direcciones aleatorias uniformemente distribuidas sobre la superficie de una esfera
 */

inline Direction muestraAleatoriaUniforme() {
    // Genera dos números aleatorios uniformes en [0, 1)
    double u = rand0_1();
    double v = rand0_1();

    // Calcula los ángulos esféricos
    double theta = acos(2.0 * u - 1.0);      // Ángulo polar
    double phi = 2.0 * M_PI * v;             // Ángulo azimutal

    // Convierte a coordenadas cartesianas
    double x = sin(theta) * cos(phi);
    double y = sin(theta) * sin(phi);
    double z = cos(theta);

    return Direction(x, y, z);
}