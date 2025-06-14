#include <iostream>
#include <chrono>
#include <vector>
#include <memory>

#include "../include/object3D.hpp"
#include "../include/parallel_renderer.hpp"
#include "../include/Image.hpp"

using namespace std;

int main() {
    // Create a simple test scene
    Scene scene;
    
    // Materials
    Material redMaterial(RGB(0.8, 0.2, 0.2), RGB(0, 0, 0)); 
    Material greenMaterial(RGB(0.2, 0.8, 0.2), RGB(0, 0, 0)); 
    Material blueMaterial(RGB(0.2, 0.2, 0.8), RGB(0, 0, 0)); 
    Material greyMaterial(RGB(0.5, 0.5, 0.5), RGB(0, 0, 0));
    
    // Add some objects
    scene.addObject(make_shared<Sphere>(Point(-0.5, 0, 0.5), 0.3, redMaterial));
    scene.addObject(make_shared<Sphere>(Point(0.5, 0, 0.5), 0.3, greenMaterial));
    scene.addObject(make_shared<Plane>(Direction(0, 1, 0), greyMaterial, 1)); // Floor
    scene.addObject(make_shared<Cylinder>(Point(0, -1, -0.2), Direction(0, 1, 0), 0.2, 0.8, blueMaterial));
    
    // Light
    scene.addLight(make_shared<PointLight>(Point(0, 0.8, 0), RGB(2, 2, 2)));
    
    // Camera (smaller resolution for faster testing)
    PinholeCamera camera(Point(0, 0, -2.5), 35, 256, 256);
    
    unsigned samples = 16; // Reduced for faster testing
    
    cout << "=== Parallel Rendering Performance Test ===\n\n";
    
    // Test sequential rendering
    cout << "Sequential rendering...\n";
    auto start = chrono::high_resolution_clock::now();
    Image sequentialImage = camera.renderPathTracing(scene, samples);
    auto end = chrono::high_resolution_clock::now();
    auto sequentialTime = chrono::duration_cast<chrono::milliseconds>(end - start);
    cout << "Sequential time: " << sequentialTime.count() / 1000.0 << " seconds\n\n";
    
    // Test different parallel configurations
    vector<ParallelConfig> configs = {
        ParallelConfig(RegionType::RECTANGLE, 16, 4),  // 4 threads, 16x16 blocks
        ParallelConfig(RegionType::RECTANGLE, 32, 4),  // 4 threads, 32x32 blocks  
        ParallelConfig(RegionType::LINE, 4, 4),        // 4 threads, 4 lines per task
        ParallelConfig(RegionType::LINE, 8, 4),        // 4 threads, 8 lines per task
        ParallelConfig(RegionType::COLUMN, 4, 4),      // 4 threads, 4 columns per task
    };
    
    cout << "Testing parallel configurations:\n";
    cout << "Config\t\t\tTime(s)\t\tSpeedup\t\tThreads\n";
    cout << "----------------------------------------------------\n";
    
    for (const auto& config : configs) {
        ParallelRenderer renderer(config);
        
        auto start = chrono::high_resolution_clock::now();
        Image parallelImage = renderer.renderPathTracing(camera, scene, samples);
        auto end = chrono::high_resolution_clock::now();
        
        auto parallelTime = chrono::duration_cast<chrono::milliseconds>(end - start);
        double timeSeconds = parallelTime.count() / 1000.0;
        double speedup = sequentialTime.count() / static_cast<double>(parallelTime.count());
        
        string regionName;
        switch (config.regionType) {
            case RegionType::PIXEL: regionName = "PIXEL"; break;
            case RegionType::LINE: regionName = "LINE"; break;
            case RegionType::COLUMN: regionName = "COLUMN"; break;
            case RegionType::RECTANGLE: regionName = "RECT"; break;
        }
        
        cout << regionName << "(" << config.regionSize << ")\t\t"
             << timeSeconds << "\t\t"
             << speedup << "x\t\t"
             << config.numThreads << "\n";
        
        // Save one parallel image for comparison
        if (config.regionType == RegionType::RECTANGLE && config.regionSize == 16) {
            parallelImage.writePPM("parallel_test_output.ppm");
        }
    }
    
    // Save sequential image
    sequentialImage.writePPM("sequential_test_output.ppm");
    
    cout << "\nTest completed! Check parallel_test_output.ppm and sequential_test_output.ppm\n";
    cout << "Images should look identical (parallel should match sequential)\n";
    
    return 0;
}
