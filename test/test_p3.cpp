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
#include "../include/scene.hpp"

using namespace std;

const std::string OUTPUT_DIR = "test_outputs/";

std::string getTimestampedFilename(const std::string& base, const std::string& ext) {
    std::time_t t = std::time(nullptr);
    std::tm tm = *std::localtime(&t);
    std::ostringstream oss;
    oss << base << "_" << std::put_time(&tm, "%Y%m%d_%H%M%S") << "." << ext;
    return oss.str();
}

Scene createTestScene(bool manyObjects = false) {
    Scene scene(RGB(0.1, 0.1, 0.1)); // Dark background
    
    // Materials
    Material redMaterial(RGB(0.8, 0.2, 0.2), RGB(0, 0, 0)); 
    Material greenMaterial(RGB(0.2, 0.8, 0.2), RGB(0, 0, 0)); 
    Material blueMaterial(RGB(0.2, 0.2, 0.8), RGB(0, 0, 0)); 
    Material greyMaterial(RGB(0.5, 0.5, 0.5), RGB(0, 0, 0));
    Material yellowMaterial(RGB(0.8, 0.8, 0.2), RGB(0, 0, 0));
    Material cyanMaterial(RGB(0.2, 0.8, 0.8), RGB(0, 0, 0));
    
    if (manyObjects) {
        // Create a scene with many objects to test acceleration structures
        for (int i = -3; i <= 3; ++i) {
            for (int j = -3; j <= 3; ++j) {
                for (int k = 0; k < 3; ++k) {
                    float x = i * 0.4f;
                    float y = k * 0.4f - 0.5f;
                    float z = j * 0.4f + 1.0f;
                    
                    Material* mat = &greyMaterial;
                    if (i % 3 == 0) mat = &redMaterial;
                    else if (j % 3 == 0) mat = &greenMaterial;
                    else if (k % 2 == 0) mat = &blueMaterial;
                    
                    scene.addObject(make_shared<Sphere>(Point(x, y, z), 0.15f, *mat));
                }
            }
        }
        cout << "Created scene with " << scene.objects.size() << " objects\n";
    } else {
        // Simple scene for basic tests
        scene.addObject(make_shared<Sphere>(Point(-0.5, 0, 0.5), 0.3, redMaterial));
        scene.addObject(make_shared<Sphere>(Point(0.5, 0, 0.5), 0.3, greenMaterial));
        scene.addObject(make_shared<Sphere>(Point(0, 0.5, 1.0), 0.2, blueMaterial));
        scene.addObject(make_shared<Plane>(Direction(0, 1, 0), greyMaterial, 1)); // Floor
        
        // Add some triangles for variety
        scene.addObject(make_shared<Triangle>(
            Point(-0.3, -0.5, 0.8), Point(0.3, -0.5, 0.8), Point(0, 0.2, 0.8),
            yellowMaterial));
    }
    
    // Lights
    scene.addLight(make_shared<PointLight>(Point(0, 1.5, 0), RGB(3, 3, 3)));
    scene.addLight(make_shared<PointLight>(Point(-1, 0.5, -0.5), RGB(1, 0.5, 0.5)));
    
    return scene;
}

void testRenderingAlgorithms(const Scene& scene, const PinholeCamera& camera, unsigned samples) {
    cout << "\n=== RENDERING ALGORITHMS TEST ===\n";
    
    vector<pair<RenderingAlgorithm, string>> algorithms = {
        {RenderingAlgorithm::RAY_TRACING, "RayTracing"},
        {RenderingAlgorithm::PATH_TRACING, "PathTracing"}
    };
    
    cout << left << setw(15) << "Algorithm" << setw(12) << "Time(s)" << endl;
    cout << "-------------------------" << endl;
    
    for (const auto& [algo, name] : algorithms) {
        auto start = chrono::high_resolution_clock::now();
        
        RenderConfig config(algo, samples, RenderingMode::SEQUENTIAL);
        Image image = camera.render(scene, config);
        
        auto end = chrono::high_resolution_clock::now();
        auto renderTime = chrono::duration_cast<chrono::milliseconds>(end - start);
        
        cout << left << setw(15) << name 
             << setw(12) << fixed << setprecision(3) << renderTime.count() / 1000.0 << endl;
             
        string filename = getTimestampedFilename("p3_" + name, "ppm");
        image.writePPM(OUTPUT_DIR + filename);
    }
}

void testParallelization(const Scene& scene, const PinholeCamera& camera, unsigned samples) {
    cout << "\n=== PARALLELIZATION TEST ===\n";
    
    // Test sequential vs parallel
    auto start = chrono::high_resolution_clock::now();
    RenderConfig sequentialConfig(RenderingAlgorithm::PATH_TRACING, samples, RenderingMode::SEQUENTIAL);
    Image sequentialImage = camera.render(scene, sequentialConfig);
    auto end = chrono::high_resolution_clock::now();
    auto sequentialTime = chrono::duration_cast<chrono::milliseconds>(end - start);
    
    // Test different parallel configurations with better granularity
    vector<RenderConfig> configs = {
        [](){ RenderConfig c; c.regionType=RegionType::RECTANGLE; c.regionSize=32; c.numThreads=2; return c; }(),  // Larger blocks for 2 threads
        [](){ RenderConfig c; c.regionType=RegionType::RECTANGLE; c.regionSize=16; c.numThreads=4; return c; }(),
        [](){ RenderConfig c; c.regionType=RegionType::LINE; c.regionSize=16; c.numThreads=4; return c; }(),        // Larger line chunks
        [](){ RenderConfig c; c.regionType=RegionType::COLUMN; c.regionSize=16; c.numThreads=4; return c; }(),      // Larger column chunks
    };
    
    cout << left << setw(20) << "Configuration" << setw(12) << "Time(s)" << setw(12) << "Speedup" << endl;
    cout << "----------------------------------------------" << endl;
    
    cout << left << setw(20) << "Sequential" 
         << setw(12) << fixed << setprecision(3) << sequentialTime.count() / 1000.0 
         << setw(12) << "1.00x" << endl;
    
    for (const auto& config : configs) {
        auto start = chrono::high_resolution_clock::now();
        Image parallelImage = ParallelRenderer::render(camera, scene, config);
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
        
        string configName = regionName + "(" + to_string(config.regionSize) + 
                           ")_T" + to_string(config.numThreads);            cout << left << setw(20) << configName
                 << setw(12) << fixed << setprecision(3) << timeSeconds
                 << setw(12) << fixed << setprecision(3) << speedup << "x";
            
            // Performance validation
            if (speedup < 0.8) {
                cout << " ⚠️ SLOWER";
            } else if (speedup > 1.5) {
                cout << " ✅ GOOD";
            }
            cout << endl;
    }
    
    // Save one parallel image
    string filename = getTimestampedFilename("p3_parallel", "ppm");
    sequentialImage.writePPM(OUTPUT_DIR + filename);
}

void testAccelerationStructures(unsigned samples) {
    cout << "\n=== ACCELERATION STRUCTURES TEST ===\n";
    
    // Test with different scene complexities
    vector<pair<bool, string>> sceneConfigs = {
        {false, "Simple(~7 objects)"},
        {true, "Complex(many objects)"}
    };
    
    PinholeCamera camera(Point(0, 0, -3), 35, 128, 128); // Smaller resolution for speed
    
    for (const auto& [isComplex, sceneName] : sceneConfigs) {
        cout << "\nTesting with " << sceneName << ":\n";
        
        // Create scene for this test
        Scene scene = createTestScene(isComplex);
        
        vector<pair<AccelerationStructure, string>> accelerations = {
            {AccelerationStructure::NONE, "None"},
            {AccelerationStructure::KDTREE, "KDTree"}
        };
        
        cout << left << setw(15) << "Acceleration" << setw(12) << "Build(s)" 
             << setw(12) << "Render(s)" << setw(15) << "Total(s)" << setw(15) << "Stats" << endl;
        cout << "-----------------------------------------------------------------------" << endl;
        
        for (const auto& [accel, name] : accelerations) {
            // Build acceleration structure
            auto buildStart = chrono::high_resolution_clock::now();
            scene.buildAccelerationStructure(accel);
            auto buildEnd = chrono::high_resolution_clock::now();
            auto buildTime = chrono::duration_cast<chrono::milliseconds>(buildEnd - buildStart);
            
            // Render with acceleration structure
            auto renderStart = chrono::high_resolution_clock::now();
            RenderConfig config(RenderingAlgorithm::RAY_TRACING, samples);
            config.acceleration = accel;
            Image image = camera.render(scene, config);
            auto renderEnd = chrono::high_resolution_clock::now();
            auto renderTime = chrono::duration_cast<chrono::milliseconds>(renderEnd - renderStart);
            
            double buildSeconds = buildTime.count() / 1000.0;
            double renderSeconds = renderTime.count() / 1000.0;
            double totalSeconds = buildSeconds + renderSeconds;
            
            cout << left << setw(15) << name
                 << setw(12) << fixed << setprecision(3) << buildSeconds
                 << setw(12) << fixed << setprecision(3) << renderSeconds
                 << setw(15) << fixed << setprecision(3) << totalSeconds
                 << setw(15) << scene.getAccelerationStats() << endl;
                 
            // Save image for the complex scene
            if (sceneName.find("Complex") != string::npos) {
                string filename = getTimestampedFilename("p3_accel_" + name, "ppm");
                image.writePPM(OUTPUT_DIR + filename);
            }
        }
    }
}

void testCombinedFeatures(const Scene&, const PinholeCamera& camera, unsigned samples) {
    cout << "\n=== COMBINED FEATURES TEST ===\n";
    cout << "Testing PATH_TRACING + PARALLEL + KDTREE acceleration\n";
    
    // Test the combination of all features
    RenderConfig config(RenderingAlgorithm::PATH_TRACING, samples, RenderingMode::PARALLEL);
    config.acceleration = AccelerationStructure::KDTREE;
    config.regionType = RegionType::RECTANGLE;
    config.regionSize = 16;
    config.numThreads = 4;
    
    // Create a new scene for this test (can't copy because of unique_ptr)
    Scene testScene = createTestScene(false);
    auto buildStart = chrono::high_resolution_clock::now();
    testScene.buildAccelerationStructure(AccelerationStructure::KDTREE);
    auto buildEnd = chrono::high_resolution_clock::now();
    
    // Render with all features combined
    auto renderStart = chrono::high_resolution_clock::now();
    Image image = ParallelRenderer::render(camera, testScene, config);
    auto renderEnd = chrono::high_resolution_clock::now();
    
    auto buildTime = chrono::duration_cast<chrono::milliseconds>(buildEnd - buildStart);
    auto renderTime = chrono::duration_cast<chrono::milliseconds>(renderEnd - renderStart);
    
    cout << "Build time: " << fixed << setprecision(3) << buildTime.count() / 1000.0 << "s\n";
    cout << "Render time: " << fixed << setprecision(3) << renderTime.count() / 1000.0 << "s\n";
    cout << "Total time: " << fixed << setprecision(3) << (buildTime.count() + renderTime.count()) / 1000.0 << "s\n";
    cout << "Acceleration stats: " << testScene.getAccelerationStats() << "\n";
    
    string filename = getTimestampedFilename("p3_combined", "ppm");
    image.writePPM(OUTPUT_DIR + filename);
    cout << "Combined features image saved as: " << filename << "\n";
}

void run_p3_tests(int argc, char* argv[]) {
    // Default values
    unsigned samples = 8;
    int width = 256, height = 256;
    
    if (argc > 1) samples = std::stoi(argv[1]);
    if (argc > 3) { width = std::stoi(argv[2]); height = std::stoi(argv[3]); }

    cout << "=== PRÁCTICA 3 COMPREHENSIVE TEST ===\n";
    cout << "Testing: Ray Tracing, Parallelization, and Acceleration Structures\n";
    cout << "Samples: " << samples << ", Resolution: " << width << "x" << height << "\n";

    // Create test scene and camera
    Scene scene = createTestScene(false);
    PinholeCamera camera(Point(0, 0, -2.5), 35, width, height);
    
    // Run all tests
    testRenderingAlgorithms(scene, camera, samples);
    testParallelization(scene, camera, samples);
    testAccelerationStructures(samples);
    testCombinedFeatures(scene, camera, samples);
    
    cout << "\n=== ALL TESTS COMPLETED ===\n";
    cout << "Check test_outputs/ directory for generated images\n";
    cout << "Expected outputs:\n";
    cout << "- p3_RayTracing_*.ppm (basic ray tracing)\n";
    cout << "- p3_PathTracing_*.ppm (path tracing with global illumination)\n";
    cout << "- p3_parallel_*.ppm (parallelization test)\n";
    cout << "- p3_accel_*.ppm (acceleration structure comparison)\n";
    cout << "- p3_combined_*.ppm (all features combined)\n";
}

int main(int argc, char* argv[]) {
    std::cout << "Running Práctica 3 comprehensive tests...\n";
    run_p3_tests(argc, argv);
    std::cout << "Práctica 3 tests completed.\n";
    return 0;
}
