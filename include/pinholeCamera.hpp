#pragma once

#include "geometry.hpp"
#include "Image.hpp"
#include "RGB.hpp"
#include "render_config.hpp"
#include "foton.hpp"
#include "kernel.hpp"
#include "object3D.hpp"

class Scene;

class PinholeCamera {
public:
    PinholeCamera() = default;

    PinholeCamera(const Point& origin) : PinholeCamera(origin, 50, 256, 256) {}

    PinholeCamera(const Point& origin, const int FOV, const int width, const int height);

    PinholeCamera(const Point& origin, const Direction& up, const Direction& left, const Direction& forward, int width, int height)
        : origin(origin), left(left), up(up), forward(forward), width(width), height(height) {
        // Store extents before normalizing
        halfExtentX = left.mod();
        halfExtentY = up.mod();
        // Then normalize
        this->left = left.normalize();
        this->up = up.normalize();
        
        calculatePixelSizes();
    }

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
    Point getOrigin() const { return origin; }
    int getWidth() const { return width; }
    int getHeight() const { return height; }
    float getFOV() const { return 2 * atan(halfExtentX) * (180.0f / M_PI); } // Returns FOV in degrees
    float getHalfExtentX() const { return halfExtentX; }
    float getHalfExtentY() const { return halfExtentY; }
    float getPixelSizeX() const { return pixelSizeX; }
    float getPixelSizeY() const { return pixelSizeY; }

    // Public methods for strategies to acces
    Ray generateRay(float x, float y) const {
        Direction direction = forward + left * x + up * y;
        return Ray(origin, direction.normalize());
    }
    RGB traceRay(const Ray& ray, const Scene& scene) const;
    RGB tracePath(const Ray& ray, const Scene& scene, unsigned depth = 0) const;

    std::string toString() const {
        return "PinholeCamera(origin: " + origin.toString() + 
               ", left: " + left.toString() + 
               ", up: " + up.toString() + 
               ", forward: " + forward.toString() + 
               ", width: " + std::to_string(width) + 
               ", height: " + std::to_string(height) + ")";
    }

    friend std::ostream& operator<<(std::ostream& os, const PinholeCamera& camera) {
        return os << camera.toString();
    }

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
