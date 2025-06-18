#pragma once

#include <vector>
#include <memory>
#include <optional>
#include <string>

#include "geometry.hpp"
#include "RGB.hpp"
#include "object3D.hpp"
#include "render_config.hpp"
#include "acceleration.hpp"
#include "pinholeCamera.hpp"

class Scene {
public:

    Scene() = default;
    Scene(const RGB& backgroundColor) : backgroundColor(backgroundColor) {}
    
    // Make Scene movable but not copyable
    Scene(const Scene&) = delete;
    Scene& operator=(const Scene&) = delete;
    Scene(Scene&&) = default;
    Scene& operator=(Scene&&) = default;

    std::vector<std::shared_ptr<Object3D>> objects;
    std::vector<std::shared_ptr<PointLight>> lights;
    RGB backgroundColor = RGB(0, 0, 0); // Color de fondo por defecto

    // Acceleration structure management
    void buildAccelerationStructure(AccelerationStructure type = AccelerationStructure::NONE);
    void setAccelerationStructure(AccelerationStructure type) {
        if (type != currentAcceleration_) {
            buildAccelerationStructure(type);
        }
    }
    AccelerationStructure getAccelerationStructure() const { return currentAcceleration_; }
    std::string getAccelerationStats() const {
        if (accelerationStructure_) {
            return accelerationStructure_->getStats();
        }
        return "No acceleration structure";
    }

    
    void addObject(const std::shared_ptr<Object3D>& object) {
        objects.push_back(object);
        accelerationBuilt_ = false; // Mark acceleration structure as needing rebuild
    }
    void addLight(const std::shared_ptr<PointLight>& light) { lights.push_back(light); }

    unsigned getPointLightCount() const { return lights.size(); }
    unsigned getLightCount() const { 
        return std::count_if(objects.begin(), objects.end(), [](const auto& object) {
            return object->material.isEmissive();
        }) + getPointLightCount();
    }
    
    // Ray intersection methods
    std::optional<Intersection> intersect(const Ray& ray, const float distance = 1000.0f) const;
    bool intersectAny(const Ray& ray, const float distance = 1000.0f) const;  // For shadow rays
    
    RGB nextEventEstimation(const Intersection& inter) const;

    // Photon map generation
    MapaFotones generarMapaFotones(const int nPaths, unsigned maxBounces) const;

    // Photon random walk following professor's specifications
    void reboteFoton(Ray currentRay,
                     RGB currentFlux,
                     std::list<Foton>& fotones,
                     std::list<Foton>& causticos,
                     bool esCaustico,
                     unsigned maxBounces) const;
    RGB ecuacionRenderFotones(Direction wo, const Intersection& intersection, MapaFotones mapa, const RenderConfig& config, const Kernel& kernel) const;
 
    std::string toString() const;
    friend std::ostream& operator<<(std::ostream& os, const Scene& scene) {
        return os << scene.toString();
    }

    // Cornell box default scene  
    static Scene& defaultScene();

    // YAML scene loader - returns scene and optional camera
    static std::optional<std::pair<Scene, std::optional<PinholeCamera>>> fromYAML(const std::string& filename);
    
    // Save scene to YAML file - camera is optional
    bool saveToYAML(const std::string& filename, const PinholeCamera* camera = nullptr) const;

private:
    // Acceleration structure management
    mutable std::unique_ptr<AccelerationStructureInterface> accelerationStructure_;
    AccelerationStructure currentAcceleration_ = AccelerationStructure::NONE;
    mutable bool accelerationBuilt_ = false;
};
