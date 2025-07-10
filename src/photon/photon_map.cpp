#include "photon/photon_map.hpp"

namespace photon {

void PhotonMap::build(const std::list<Photon>& photons) {
    photons_ = photons;
    if (!photons_.empty()) {
        kdTree_ = PhotonKDTree(photons_, PhotonPositionAccessor());
        built_ = true;
    } else {
        built_ = false;
    }
}

std::vector<Photon> PhotonMap::findNearestPhotons(const Point& queryPoint, 
                                                   unsigned long maxPhotons,
                                                   float maxRadius) const {
    std::vector<Photon> result;
    
    if (!built_ || photons_.empty()) {
        return result;
    }
    
    // Query the KD-tree for nearest photons
    auto nearest = kdTree_.nearest_neighbors(queryPoint, maxPhotons, maxRadius);
    
    // Convert the results to our return format
    result.reserve(nearest.size());
    for (const auto* photonPtr : nearest) {
        result.push_back(*photonPtr);
    }
    
    return result;
}

void PhotonMap::clear() {
    photons_.clear();
    kdTree_ = PhotonKDTree();
    built_ = false;
}

} // namespace photon