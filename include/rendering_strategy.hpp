#pragma once

#include <memory>
#include "render_config.hpp"
#include "RGB.hpp"

// Forward declarations
class PinholeCamera;
class Scene;

class RenderingStrategy {
public:
    virtual ~RenderingStrategy() = default;
    
    virtual RGB calculatePixelColor(const PinholeCamera& camera, const Scene& scene, 
                                   float x, float y, unsigned samples, 
                                   const RenderConfig& config) const = 0;
};

class RayTracingStrategy : public RenderingStrategy {
public:
    RGB calculatePixelColor(const PinholeCamera& camera, const Scene& scene, 
                           float x, float y, unsigned samples, 
                           const RenderConfig& config) const override;
};

class PathTracingStrategy : public RenderingStrategy {
public:
    RGB calculatePixelColor(const PinholeCamera& camera, const Scene& scene, 
                           float x, float y, unsigned samples, 
                           const RenderConfig& config) const override;
};

class PhotonMappingStrategy : public RenderingStrategy {
public:
    RGB calculatePixelColor(const PinholeCamera& camera, const Scene& scene, 
                           float x, float y, unsigned samples, 
                           const RenderConfig& config) const override;
};

class StrategyFactory {
public:
    static std::unique_ptr<RenderingStrategy> createStrategy(RenderingAlgorithm algorithm);
};
