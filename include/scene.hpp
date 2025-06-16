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
    
    // Ray intersection methods
    std::optional<Intersection> intersect(const Ray& ray, const float distance = 1000.0f) const;
    bool intersectAny(const Ray& ray, const float distance = 1000.0f) const;  // For shadow rays
    
    RGB calculateDirectLight(const Point& p) const;
    MapaFotones generarMapaFotones(int nPaths, bool save, double sigma = 0.0f) const;
    void reboteFoton(const Ray& ray, const RGB& light, std::list<Foton>& fotones, std::list<Foton>& causticos, bool esCaustico, bool save = false, double sigma = 0.0f) const;
    RGB ecuacionRenderFotones(Point x, Direction wo, Material material, Direction n, MapaFotones mapa, int kFotones, double radio, bool guardar, Kernel* kernel, double sigma = 0.0f) const;
    RGB estimacionSiguienteEvento(Point point, Direction wo, Material material, Direction n, double sigma) const;
 
    void sortObjectsByDistanceToCamera(const Point& cameraPosition);
    std::string toString() const;
    friend std::ostream& operator<<(std::ostream& os, const Scene& scene) {
        return os << scene.toString();
    }

    static Scene& defaultScene() { // Cornell box
        static Scene scene(RGB(0.2f, 0.2f, 0.2f)); // Default background color
        
        Material redMaterial(RGB(1, 0, 0), RGB(1, 1, 1), false); // Red material
        Material greenMaterial(RGB(0, 1, 0), RGB(1, 1, 1), false); // Green material
        Material greyMaterial(RGB(0.5, 0.5, 0.5), RGB(1, 1, 1), false); // Grey material
        Material plasticMaterial(RGB(1, 1, 1), RGB(1, 1, 1), false); // Plastic material

        scene.addObject(std::make_shared<Plane>(Direction(1, 0, 0), redMaterial)); // Right plane (Green)
        scene.addObject(std::make_shared<Plane>(Direction(-1, 0, 0), greenMaterial)); // Left plane (Red)
        scene.addObject(std::make_shared<Plane>(Direction(0, 1, 0), greyMaterial)); // Floor plane 
        scene.addObject(std::make_shared<Plane>(Direction(0, -1, 0), greyMaterial)); // Ceiling plane 
        scene.addObject(std::make_shared<Plane>(Direction(0, 0, 1), greyMaterial)); // Back plane 

        // Spheres-
        // Estas intersecciones funcionan pero se detecta rarote
        scene.addObject(std::make_shared<Sphere>(Point(-0.5, 0.7, 0.25), 0.3, plasticMaterial)); // Left sphere
        scene.addObject(std::make_shared<Sphere>(Point(0.5, 0.7, -0.25), 0.3, plasticMaterial)); // Right sphere

        // Lights
        auto shared_pointLight = std::make_shared<PointLight>(Point(0, 0.15, 0), RGB(1, 1, 1)); // Light source
        scene.addLight(shared_pointLight); // Light source

        return scene;
    }

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
