#pragma once

#include "geometry.hpp"
#include "Image.hpp"
#include "RGB.hpp"
#include "render_config.hpp"
#include "foton.hpp"
#include "kernel.hpp"
#include "object3D.hpp"

class PinholeCamera {
public:
 
    PinholeCamera(const Point& origin) : PinholeCamera(origin, 50, 256, 256) {}

    PinholeCamera(const Point& origin, const int FOV, const int width, const int height);

    PinholeCamera(const Point& origin, const Direction& up, const Direction& left, const Direction& forward, int width, int height)
        : origin(origin), left(left.normalize()), up(up.normalize()), forward(forward), width(width), height(height) {}

    // Main unified render method
    Image render(const Scene& scene, unsigned samplesPerPixel, 
                const RenderConfig& config = RenderConfig{}) const;
    
    // Convenience methods (thin wrappers for backward compatibility)
    Image renderPathTracing(const Scene& scene, unsigned samples, const RenderConfig& rc = RenderConfig{RenderingAlgorithm::PATH_TRACING}) const {
        return render(scene, samples, rc);
    }
    
    Image renderRayTracing(const Scene& scene, unsigned samples, const RenderConfig& rc = RenderConfig{RenderingAlgorithm::RAY_TRACING}) const {
        return render(scene, samples, rc);
    }
    
    Image renderPhotonMapping(const Scene& scene, unsigned samples, 
                MapaFotones mapa, unsigned kPhotons, double radio, Kernel* kernel) const {
        RenderConfig config{RenderingAlgorithm::PHOTON_MAPPING};
        config.photonMap = &mapa;
        config.kPhotons = kPhotons;
        config.radius = radio;
        config.kernel = kernel;
        return render(scene, samples, config);
    }

    // Accessors
    int getWidth() const { return width; }
    int getHeight() const { return height; }

    // Public methods for strategies to access
    Ray generateRay(float x, float y) const;
    RGB traceRay(const Ray& ray, const Scene& scene) const;
    RGB tracePath(const Ray& ray, const Scene& scene, unsigned depth = 0) const;

private:
    Point origin;
    Direction left, up, forward;
    int width, height;
};
