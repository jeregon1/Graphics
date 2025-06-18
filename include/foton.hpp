#pragma once

#include <list>
#include "geometry.hpp"
#include "RGB.hpp"
#include "kdtree.h"

class Foton {
    public:
    Point position;
    Direction incidentDir;
    RGB flux;

    static constexpr int MAX_BOUNCES = 10;

    Foton(Point p, Direction d, RGB f) : position(p), incidentDir(d), flux(f) {}
    ~Foton() {}

    double getPosition(std::size_t i) const {
        switch(i) {
            case 0: return position.x;
            case 1: return position.y;
            case 2: return position.z;
            default: throw std::out_of_range("Index out of range for Point");
        }
    }
};

struct PosicionEjeFoton {
    float operator()(const Foton& p, std::size_t i) const {
        return p.getPosition(i);
    }
};

using MapaFotones = nn::KDTree<Foton,3,PosicionEjeFoton>;

inline MapaFotones construirMapaFotones(std::list<Foton> fotones) {
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
