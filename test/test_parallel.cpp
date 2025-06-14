#include <iostream>
#include <chrono>
#include <vector>
#include <memory>
#include <ctime>
#include <iomanip>
#include <sstream>

#include "../include/object3D.hpp"
#include "../include/pinholeCamera.hpp"
#include "../include/parallel_renderer.hpp"
#include "../include/Image.hpp"

using namespace std;

const std::string OUTPUT_DIR = "test_outputs/";

std::string getTimestampedFilename(const std::string& base, const std::string& ext) {
    std::time_t t = std::time(nullptr);
    std::tm tm = *std::localtime(&t);
    std::ostringstream oss;
    oss << base << "_" << std::put_time(&tm, "%Y%m%d_%H%M%S") << "." << ext;
    return oss.str();
}

void run_parallel_tests(int argc, char* argv[]) {
    // Default values
    unsigned samples = 16;
    int width = 256, height = 256;
    std::string outputBase = "parallel_test_output";
    if (argc > 1) samples = std::stoi(argv[1]);
    if (argc > 3) { width = std::stoi(argv[2]); height = std::stoi(argv[3]); }
    if (argc > 4) outputBase = argv[4];

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
    PinholeCamera camera(Point(0, 0, -2.5), 35, width, height);
    
    std::string parallelOutput = getTimestampedFilename(outputBase, "ppm");
    std::string sequentialOutput = getTimestampedFilename("sequential_test_output", "ppm");

    cout << "=== Parallel Rendering Performance Test ===\n\n";
    
    // Test sequential rendering
    cout << "Sequential rendering...\n";
    auto start = chrono::high_resolution_clock::now();
    RenderConfig sequentialConfig(RenderingAlgorithm::PATH_TRACING, RenderingMode::SEQUENTIAL);
    Image sequentialImage = camera.renderPathTracing(scene, samples, sequentialConfig);
    auto end = chrono::high_resolution_clock::now();
    auto sequentialTime = chrono::duration_cast<chrono::milliseconds>(end - start);
    cout << "Sequential time: " << sequentialTime.count() / 1000.0 << " seconds\n";
    sequentialImage.writePPM(OUTPUT_DIR + sequentialOutput);

    // Test different parallel configurations
    vector<RenderConfig> configs = {
        [](){ RenderConfig c; c.regionType=RegionType::RECTANGLE; c.regionSize=16; c.numThreads=4; return c; }(),
        [](){ RenderConfig c; c.regionType=RegionType::RECTANGLE; c.regionSize=32; c.numThreads=4; return c; }(),
        [](){ RenderConfig c; c.regionType=RegionType::LINE; c.regionSize=4; c.numThreads=4; return c; }(),
        [](){ RenderConfig c; c.regionType=RegionType::LINE; c.regionSize=16; c.numThreads=4; return c; }(),
        [](){ RenderConfig c; c.regionType=RegionType::COLUMN; c.regionSize=4; c.numThreads=4; return c; }(),
    };
    
    cout << "Testing parallel configurations:\n";
    cout << left << setw(15) << "Config" << setw(12) << "Time(s)" << setw(12) << "Speedup" << setw(10) << "Threads" << endl;
    cout << "-----------------------------------------------" << endl;
    
    Image savedParallelImage;
    bool imageSaved = false;
    
    for (size_t i = 0; i < configs.size(); ++i) {
        const auto& config = configs[i];
        ParallelRenderer renderer(config);
        auto start = chrono::high_resolution_clock::now();
        Image parallelImage = renderer.render(camera, scene, samples, RenderConfig());
        auto end = chrono::high_resolution_clock::now();
        auto parallelTime = chrono::duration_cast<chrono::milliseconds>(end - start);
        double timeSeconds = parallelTime.count() / 1000.0;
        double speedup = sequentialTime.count() / static_cast<double>(parallelTime.count());
        
        string regionName;
        switch (config.regionType) {
            case RegionType::PIXEL: regionName = "PIXEL"; break;
            case RegionType::LINE: regionName = "LINE "; break;
            case RegionType::COLUMN: regionName = "COLUMN"; break;
            case RegionType::RECTANGLE: regionName = "RECT"; break;
        }
        
        cout << left << setw(15) << (regionName + "(" + to_string(config.regionSize) + ")")
             << setw(12) << fixed << setprecision(3) << timeSeconds
             << setw(12) << fixed << setprecision(3) << speedup << "x"
             << setw(10) << config.numThreads << endl;
             
        // Save one parallel image for comparison (but don't write yet)
        if (config.regionType == RegionType::RECTANGLE && config.regionSize == 16 && !imageSaved) {
            savedParallelImage = parallelImage;
            imageSaved = true;
        }
    }
    
    // Write the saved image after all tests
    if (imageSaved) {
        savedParallelImage.writePPM(OUTPUT_DIR + parallelOutput);
    }
    cout << std::endl;
    
    cout << "\nTest completed! Check " << parallelOutput << " and " << sequentialOutput << std::endl;
    cout << "Images should look identical (parallel should match sequential)\n";
}
