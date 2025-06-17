#pragma once

#include <random>
#include <cmath>
#include <cstdlib>
#include "geometry.hpp"

//https:projecteuclid.org/journals/annals-of-mathematical-statistics/volume-43/issue-2/Choosing-a-Point-from-the-Surface-of-a-Sphere/10.1214/aoms/1177692644.full

namespace utils {
    static std::mt19937 rng(std::random_device{}());
    static std::uniform_real_distribution<double> dist01(0.0, 1.0);
    
    inline float cosTheta(const Direction& a, const Direction& b) {
        return std::max(0.0f, a.dot(b));
    }
}

inline double rand0_1() {
    return utils::dist01(utils::rng);
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

/*
 * Generates a random direction on the hemisphere defined by the normal vector.
 * This is used for path tracing to sample directions uniformly with cosine weighting.
 */
// https://the-last-stand.github.io/ray-tracing-practice/the_rest_of_your_life/generating_random_directions/
inline Direction randomCosineDirection(const Direction& normal) {
    float r1 = 2 * M_PI * rand0_1();
    float r2 = rand0_1();
    float r2s = sqrt(r2);

    // Base ortonormal
    Direction w = normal;
    // Vectores perpendiculares a w
    Direction u = ((fabs(w.x) > 0.1 ? Direction(0,1,0) : Direction(1,0,0)).cross(w)).normalize();
    Direction v = w.cross(u);

    return (u * cos(r1) * r2s + v * sin(r1) * r2s + w * sqrt(1 - r2)).normalize();
}