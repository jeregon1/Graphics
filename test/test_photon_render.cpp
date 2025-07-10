#include <iostream>
#include <fstream>
#include <chrono>
#include "scene.hpp"
#include "pinholeCamera.hpp"
#include "parallel_renderer.hpp"
#include "render_config.hpp"
#include "kernel.hpp"

int main() {
    std::cout << "Testing photon mapping rendering with modular system..." << std::endl;
    
    // Create a Cornell box-like scene
    Scene scene;
    scene.backgroundColor = RGB(0.1, 0.1, 0.1);
    
    // Add lights
    auto light1 = std::make_shared<PointLight>(Point(0, 4, 0), RGB(50, 50, 50));
    scene.addLight(light1);
    
    // Add walls and floor
    Material wallMaterial(RGB(0.8, 0.8, 0.8));
    Material redMaterial(RGB(0.8, 0.2, 0.2));
    Material greenMaterial(RGB(0.2, 0.8, 0.2));
    
    // Floor
    auto floor = std::make_shared<Plane>(Direction(0, 1, 0), wallMaterial, 2.0f);
    scene.addObject(floor);
    
    // Ceiling
    auto ceiling = std::make_shared<Plane>(Direction(0, -1, 0), wallMaterial, 5.0f);
    scene.addObject(ceiling);
    
    // Left wall (red)
    auto leftWall = std::make_shared<Plane>(Direction(1, 0, 0), redMaterial, 3.0f);
    scene.addObject(leftWall);
    
    // Right wall (green)  
    auto rightWall = std::make_shared<Plane>(Direction(-1, 0, 0), greenMaterial, 3.0f);
    scene.addObject(rightWall);
    
    // Back wall
    auto backWall = std::make_shared<Plane>(Direction(0, 0, -1), wallMaterial, 6.0f);
    scene.addObject(backWall);
    
    // Add a sphere
    Material sphereMaterial(RGB(0.6, 0.6, 0.6));
    auto sphere = std::make_shared<Sphere>(Point(-1, 0, 3), 1.0f, sphereMaterial);
    scene.addObject(sphere);
    
    // Set up camera
    PinholeCamera camera(Point(0, 1, -1), 60, 256, 256, Direction(0, 0, 1));
    
    // Configure render settings for photon mapping
    RenderConfig config;
    config.algorithm = RenderingAlgorithm::PHOTON_MAPPING;
    config.samplesPerPixel = 2;
    config.maxBounces = 5;
    config.nPaths = 5000;  // Number of photons to generate
    config.kPhotons = 50;  // Number of photons to gather for radiance estimation
    config.radius = 0.5f;  // Search radius for photon gathering
    config.kernel = nullptr; // Use default kernel
    
    std::cout << "Rendering with photon mapping..." << std::endl;
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // Render the scene using the camera's render method
    Image image = camera.render(scene, config);
    
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    
    std::cout << "Rendering completed in " << duration.count() << " ms" << std::endl;
    
    // Save the image
    std::string filename = "photon_mapping_test.ppm";
    image.writePPM(filename);
    std::cout << "Image saved as: " << filename << std::endl;
    
    std::cout << "Photon mapping rendering test completed!" << std::endl;
    return 0;
}