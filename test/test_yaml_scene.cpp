#include <iostream>
#include <string>
#include "../include/object3D.hpp"

int main() {
    std::cout << "=== YAML Scene Test ===" << std::endl;
    
    try {
        // Load scene from nested-material YAML file
        Scene scene = Scene::fromYAML("../cornell_scene.yaml");
         
        std::cout << "Successfully loaded scene from cornell_scene.yaml" << std::endl << std::endl;
        
        // Print summary scene information
        std::cout << scene.toString() << std::endl;
        
        // Detailed print of objects and lights to verify nested material parsing
        std::cout << "---- Objects ----" << std::endl;
        for (const auto& obj : scene.objects) {
            std::cout << obj->toString() << std::endl;
        }
        std::cout << "---- Lights ----" << std::endl;
        for (const auto& light : scene.lights) {
            std::cout << light->toString() << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error loading scene: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
