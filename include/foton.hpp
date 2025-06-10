#pragma once

#include <list>
#include "geometry.hpp"
#include "RGB.hpp"
#include "kdtree.h"

// TODO: Refactorizar los nombres de esta clase
class Foton {
    public:
    Point posicion;
    Direction direccion;
    RGB flujo;

    Foton(Point p, Direction d, RGB f) : posicion(p), direccion(d), flujo(f) {}
    ~Foton() {}

    double position(std::size_t i) const {
        switch(i) {
            case 0: return posicion.x;
            case 1: return posicion.y;
            case 2: return posicion.z;
            default: throw std::out_of_range("Index out of range for Point");
        }
    }
};

struct PosicionEjeFoton {
    float operator()(const Foton& p, std::size_t i) const {
        return p.position(i);
    }
};

using MapaFotones = nn::KDTree<Foton,3,PosicionEjeFoton>;

MapaFotones construirMapaFotones(std::list<Foton> fotones) {
    return MapaFotones(fotones, PosicionEjeFoton());
}

/*
void search_nearest(MapaFotones map, ...){
    // Position to look for the nearest photons
    Point query_position = ...;    

    // Maximum number of photons to look for
    unsigned long nphotons_estimate = ...;

    // Maximum distance to look for photons
    float radius_estimate = ...;

    // nearest is the nearest photons returned by the KDTree
    auto nearest = map.nearest_neighbors(query_position,
                                         nphotons_estimate,
                                         radius_estimate)
}
                                         
*/
