#pragma once

#include "geometry.hpp"
#include "Image.hpp"
#include "RGB.hpp"
#include "render_config.hpp"
#include "foton.hpp"
#include "kernel.hpp"
#include "object3D.hpp"
#include "rendering_strategy.hpp"
#include <iomanip>
#include <atomic>

class Scene;

class PinholeCamera {
public:
    PinholeCamera(const Point& origin = Point(0,0,-3), const int FOV = 50, const int width = 256, const int height = 256, const Direction& forward = Direction(0, 0, 1));

    PinholeCamera(const Point& origin, const Direction& up, const Direction& left, const Direction& forward, int width, int height);

    // Main unified render method
    Image render(const Scene& scene, const RenderConfig& config = RenderConfig{}) const;
    
    // Convenience methods (thin wrappers for backward compatibility)
    Image renderPathTracing(const Scene& scene, const RenderConfig& config = RenderConfig{RenderingAlgorithm::PATH_TRACING}) const {
        return render(scene, config);
    }
    
    Image renderRayTracing(const Scene& scene, const RenderConfig& config = RenderConfig{RenderingAlgorithm::RAY_TRACING}) const {
        return render(scene, config);
    }
    
    Image renderPhotonMapping(const Scene& scene, MapaFotones mapa, unsigned kPhotons, double radio, Kernel* kernel, RenderConfig config) const {
        config.algorithm = RenderingAlgorithm::PHOTON_MAPPING;
        config.photonMap = &mapa;
        config.kPhotons = kPhotons;
        config.radius = radio;
        config.kernel = kernel;
        return render(scene, config);
    }

    // Accessors
    Point getOrigin() const { return origin; }
    int getWidth() const { return width; }
    int getHeight() const { return height; }
    Direction getForward() const { return forward; }
    float getFOV() const { return 2 * atan(halfExtentX) * (180.0f / M_PI); } // Returns FOV in degrees
    float getHalfExtentX() const { return halfExtentX; }
    float getHalfExtentY() const { return halfExtentY; }
    float getPixelSizeX() const { return pixelSizeX; }
    float getPixelSizeY() const { return pixelSizeY; }
    
    // Pixel counter method
    void showProgressIfNeeded(bool verbose) const;

    // Public methods for strategies to access
    Ray generateRay(float x, float y) const {
        Direction direction = forward + left * x + up * y;
        return Ray(origin, direction.normalize());
    }
    RGB traceRay(const Ray& ray, const Scene& scene) const;
    RGB tracePath(const Ray& ray, const Scene& scene, const int bouncesLeft = 6) const;

    std::string toString() const {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2);
        oss << "PinholeCamera(origin: " << origin.toString()
            << ", FOV: " << getFOV()
            << ", forward: " << forward.toString()
            << ", pixels: " << width << "x" << height << ")";
        return oss.str();
    }

    friend std::ostream& operator<<(std::ostream& os, const PinholeCamera& camera) {
        return os << camera.toString();
    }

    // Shared pixel rendering logic for both serial and parallel
    void renderRegion(std::vector<RGB>& pixels, const Scene& scene, const RenderConfig& config, 
        int startY = 0, int startX = 0, int endY = -1, int endX = -1) const;
    
    // Render region with separate direct and indirect lighting
    void renderRegionWithLightingDecomposition(std::vector<RGB>& directPixels, std::vector<RGB>& indirectPixels, 
        const Scene& scene, const RenderConfig& config, 
        int startY = 0, int startX = 0, int endY = -1, int endX = -1) const;

private:
    Point origin;
    Direction left, up, forward;
    int width, height;
    float halfExtentX, halfExtentY; // Actual viewport half-extents
    float pixelSizeX, pixelSizeY;   // Pre-calculated pixel sizes for anti-aliasing

    void calculatePixelSizes() {
        pixelSizeX = (2.0f * halfExtentX) / width;
        pixelSizeY = (2.0f * halfExtentY) / height;
    }
};
