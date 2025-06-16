#include "../include/scene.hpp"
#include "../include/pinholeCamera.hpp"
#include "../include/render_config.hpp"
#include <iostream>
#include <filesystem>

int main() {
    std::cout << "=== YAML Consolidation Test ===" << std::endl;
    
    // Test 1: RenderConfig YAML
    std::cout << "\n--- Testing RenderConfig YAML ---" << std::endl;
    auto configOpt = RenderConfig::fromYAML("configs/default_config.yaml");
    if (!configOpt) {
        std::cerr << "Failed to load render config" << std::endl;
        return 1;
    }
    
    RenderConfig config = *configOpt;
    std::cout << "✓ Loaded config: " << config << std::endl;
    
    // Save modified config
    config.numThreads = 8;
    config.regionSize = 16;
    config.saveToYAML("test_outputs/test_output.yaml");
    std::cout << "✓ Saved modified render config" << std::endl;
    
    // Test 2: Load a scene with camera from YAML
    std::cout << "\n--- Testing Scene fromYAML ---" << std::endl;
    
    auto result = Scene::fromYAML("scenes/simple_test.yaml");
    if (result) {
        auto& [scene, camera] = *result;
        std::cout << "✓ Successfully loaded scene with " << scene.objects.size() << " objects and " 
                  << scene.lights.size() << " lights\n";
        if (camera) {
            std::cout << "✓ Camera loaded: " << camera->getWidth() << "x" << camera->getHeight() << "\n";
        } else {
            std::cout << "• No camera in scene\n";
        }
        
        // Test 3: Save scene without camera
        std::cout << "\n--- Testing Scene saveToYAML without camera ---" << std::endl;
        scene.saveToYAML("test_outputs/test_scene_only.yaml");
        
        // Test 4: Save scene with camera (if we have one)
        if (camera) {
            std::cout << "\n--- Testing Scene saveToYAML with camera ---" << std::endl;
            scene.saveToYAML("test_outputs/test_scene_with_camera.yaml", &(*camera));
        } else {
            std::cout << "\n--- Skipping camera save test (no camera available) ---" << std::endl;
        }
        
        // Test 5: Verify the saved files exist
        std::cout << "\n--- Verifying saved files ---" << std::endl;
        if (std::filesystem::exists("test_outputs/test_scene_only.yaml")) {
            std::cout << "✓ Scene-only file created successfully\n";
        } else {
            std::cout << "✗ Scene-only file not found\n";
        }
        
        if (camera && std::filesystem::exists("test_outputs/test_scene_with_camera.yaml")) {
            std::cout << "✓ Scene-with-camera file created successfully\n";
        }
        
        if (std::filesystem::exists("test_outputs/test_output.yaml")) {
            std::cout << "✓ RenderConfig file created successfully\n";
        } else {
            std::cout << "✗ RenderConfig file not found\n";
        }
        
    } else {
        std::cout << "✗ Failed to load scene from YAML\n";
        return 1;
    }
    
    std::cout << "\n✓ All YAML consolidation tests completed successfully!" << std::endl;
    return 0;
}
