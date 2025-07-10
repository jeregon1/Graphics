#pragma once

#include <list>
#include <vector>
#include "geometry.hpp"
#include "RGB.hpp"
#include "kdtree.h"

namespace photon {

class Photon {
public:
    Point position;
    Direction incidentDir;
    RGB flux;

    Photon(Point p, Direction d, RGB f) : position(p), incidentDir(d), flux(f) {}
    ~Photon() {}

    double getPosition(std::size_t i) const {
        switch(i) {
            case 0: return position.x;
            case 1: return position.y;
            case 2: return position.z;
            default: throw std::out_of_range("Index out of range for Point");
        }
    }
};

struct PhotonPositionAccessor {
    float operator()(const Photon& p, std::size_t i) const {
        return p.getPosition(i);
    }
};

using PhotonKDTree = nn::KDTree<Photon, 3, PhotonPositionAccessor>;

class PhotonMap {
public:
    PhotonMap() = default;
    ~PhotonMap() = default;

    // Build the photon map from a list of photons
    void build(const std::list<Photon>& photons);
    
    // Query nearest photons within a radius
    std::vector<Photon> findNearestPhotons(const Point& queryPoint, 
                                           unsigned long maxPhotons,
                                           float maxRadius) const;
    
    // Check if the photon map is built
    bool isBuilt() const { return built_; }
    
    // Get the number of photons in the map
    size_t size() const { return photons_.size(); }
    
    // Clear the photon map
    void clear();

private:
    std::list<Photon> photons_;
    PhotonKDTree kdTree_;
    bool built_ = false;
};

} // namespace photon