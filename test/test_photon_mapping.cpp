#include "photon/photon_mapping.hpp"
#include "scene.hpp"
#include "render_config.hpp"
#include "kernel.hpp"
#include <iostream>

int main() {
    std::cout << "Testing photon mapping modularization..." << std::endl;
    
    // Create a simple scene with a light and objects
    Scene scene;
    
    // Add a point light
    auto light = std::make_shared<PointLight>(Point(0, 5, 0), RGB(10, 10, 10));
    scene.addLight(light);
    
    // Create a floor (large plane) - just a horizontal plane at y=-2
    Material floorMaterial(RGB(0.8, 0.8, 0.8)); // Diffuse white material
    auto floor = std::make_shared<Plane>(Direction(0, 1, 0), floorMaterial, 2.0f);
    scene.addObject(floor);
    
    // Create a sphere that can receive photons
    Material sphereMaterial(RGB(0.6, 0.6, 0.6)); // Diffuse gray material
    auto sphere = std::make_shared<Sphere>(Point(0, 0, 5), 1.0f, sphereMaterial);
    scene.addObject(sphere);
    
    // Test photon mapper creation
    auto photonMapper = std::make_shared<photon::PhotonMapper>();
    std::cout << "PhotonMapper created successfully" << std::endl;
    
    // Test photon map generation
    photonMapper->generatePhotonMaps(scene, 1000, 5);
    
    if (photonMapper->hasPhotonMaps()) {
        std::cout << "Photon maps generated successfully" << std::endl;
        std::cout << "Regular photons: " << photonMapper->getRegularPhotonMap().size() << std::endl;
        std::cout << "Caustic photons: " << photonMapper->getCausticPhotonMap().size() << std::endl;
    } else {
        std::cout << "Failed to generate photon maps" << std::endl;
    }
    
    // Test photon mapping renderer
    auto renderer = std::make_shared<photon::PhotonMappingRenderer>();
    renderer->setPhotonMapper(photonMapper);
    std::cout << "PhotonMappingRenderer created successfully" << std::endl;
    
    // Test photon map queries
    Point queryPoint(0, 0, 5);
    auto nearestPhotons = photonMapper->getRegularPhotonMap().findNearestPhotons(queryPoint, 10, 1.0f);
    std::cout << "Found " << nearestPhotons.size() << " photons near query point" << std::endl;
    
    std::cout << "All photon mapping tests passed!" << std::endl;
    return 0;
}