#pragma once

#include <random>
#include "geometry.hpp"

//https:projecteuclid.org/journals/annals-of-mathematical-statistics/volume-43/issue-2/Choosing-a-Point-from-the-Surface-of-a-Sphere/10.1214/aoms/1177692644.full

/* 
 *  Este código implementa una función para muestrear direcciones aleatorias uniformemente distribuidas sobre la superficie de una esfera
 */

Direction muestraAleatoriaUniforme() {
    // Genera dos números aleatorios uniformes en [0, 1)
    double u = static_cast<double>(rand()) / RAND_MAX;
    double v = static_cast<double>(rand()) / RAND_MAX;

    // Calcula los ángulos esféricos
    double theta = acos(2.0 * u - 1.0);      // Ángulo polar
    double phi = 2.0 * M_PI * v;             // Ángulo azimutal

    // Convierte a coordenadas cartesianas
    double x = sin(theta) * cos(phi);
    double y = sin(theta) * sin(phi);
    double z = cos(theta);

    return Direction(x, y, z);
}