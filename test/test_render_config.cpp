#include <iostream>
#include "../include/render_config.hpp"
#include "../include/scene.hpp"

int main() {
    std::cout << "=== YAML Save/Load Test ===" << std::endl;
    
    // Test render config YAML
    std::cout << "\n--- Testing RenderConfig YAML ---" << std::endl;
    auto configOpt = RenderConfig::fromYAML("configs/default_render.yaml");
    if (!configOpt) {
        std::cerr << "Failed to load render config" << std::endl;
        return 1;
    }
    
    RenderConfig config = *configOpt;
    std::cout << "Loaded config: " << config << std::endl;
    
    // Save modified config
    config.numThreads = 8;
    config.regionSize = 16;
    config.saveToYAML("configs/test_output.yaml");
    
    // Test scene YAML saving
    std::cout << "\n--- Testing Scene YAML saving ---" << std::endl;
    Scene scene;
    
    // Save scene only
    scene.saveToYAML("test_outputs/saved_scene.yaml");
    
    // Save scene with camera
    PinholeCamera camera(Point(0, 0, -3), 50, 512, 512);
    scene.saveToYAML("test_outputs/saved_scene_with_camera.yaml", camera);
    
    std::cout << "\nAll tests completed successfully!" << std::endl;
    return 0;
}
