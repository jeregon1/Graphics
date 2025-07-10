#pragma once

#include <memory>
#include <list>
#include "photon_map.hpp"
#include "geometry.hpp"
#include "RGB.hpp"

// Forward declarations
class Scene;
class PointLight;
class RenderConfig;
class Ray;
class Intersection;

namespace photon {

class PhotonMapper {
public:
    PhotonMapper() = default;
    ~PhotonMapper() = default;

    // Generate photon maps for a scene
    void generatePhotonMaps(const Scene& scene, 
                           int numPhotons, 
                           unsigned maxBounces);
    
    // Get the regular photon map
    const PhotonMap& getRegularPhotonMap() const { return regularPhotonMap_; }
    
    // Get the caustic photon map
    const PhotonMap& getCausticPhotonMap() const { return causticPhotonMap_; }
    
    // Check if photon maps are built
    bool hasPhotonMaps() const { return mapsBuilt_; }
    
    // Clear all photon maps
    void clear();

private:
    // Generate photons from a single light source
    void generatePhotonsFromLight(const PointLight& light,
                                  int numPhotons,
                                  double totalEmission,
                                  unsigned maxBounces,
                                  const Scene& scene,
                                  std::list<Photon>& regularPhotons,
                                  std::list<Photon>& causticPhotons);
    
    // Trace a photon through the scene
    void tracePhoton(const Ray& ray,
                     const RGB& flux,
                     const Scene& scene,
                     std::list<Photon>& regularPhotons,
                     std::list<Photon>& causticPhotons,
                     bool isCaustic,
                     unsigned maxBounces) const;

    PhotonMap regularPhotonMap_;
    PhotonMap causticPhotonMap_;
    bool mapsBuilt_ = false;
};

} // namespace photon