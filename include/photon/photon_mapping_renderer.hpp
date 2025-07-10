#pragma once

#include <memory>
#include "photon_mapper.hpp"
#include "RGB.hpp"
#include "geometry.hpp"

// Forward declarations
class Scene;
class Intersection;
class RenderConfig;
class Kernel;

namespace photon {

class PhotonMappingRenderer {
public:
    PhotonMappingRenderer() = default;
    ~PhotonMappingRenderer() = default;

    // Set the photon mapper to use
    void setPhotonMapper(std::shared_ptr<PhotonMapper> mapper) { 
        photonMapper_ = mapper; 
    }
    
    // Render a pixel using photon mapping
    RGB renderPixel(const Direction& viewDirection,
                    const Intersection& intersection,
                    const Scene& scene,
                    const RenderConfig& config,
                    const Kernel& kernel,
                    int maxBounces) const;

private:
    // Estimate radiance using photon mapping
    RGB estimateRadiance(const Direction& viewDirection,
                         const Intersection& intersection,
                         const Scene& scene,
                         const RenderConfig& config,
                         const Kernel& kernel,
                         int maxBounces) const;
    
    // Handle direct lighting estimation
    RGB estimateDirectLighting(const Intersection& intersection,
                               const Scene& scene) const;

    std::shared_ptr<PhotonMapper> photonMapper_;
};

} // namespace photon