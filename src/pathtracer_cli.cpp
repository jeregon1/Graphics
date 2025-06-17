/*
 * pathtracer_cli.cpp
 * Author: Path tracer CLI with reflectance properties
 * Description: Command-line path tracer with reflectance support and HDR output
 */

#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <memory>

#include "../include/pinholeCamera.hpp"
#include "../include/scene.hpp"
#include "../include/Image.hpp"
#include "../include/render_config.hpp"
#include "../include/parallel_renderer.hpp"

using namespace std;

void printUsage(const string& programName) {
    cout << "Usage: " << programName << " <scene_file> <output_file> <samples> [width] [height] [color_resolution]\n"
         << "\nParameters:\n"
         << "  scene_file        - YAML scene file to render (or \"default\")\n"
         << "  output_file       - Output image filename (.ppm or .bmp)\n"
         << "  samples          - Number of samples per pixel (path tracing quality)\n"
         << "  width            - Image width in pixels (default: 512)\n"
         << "  height           - Image height in pixels (default: 512)\n"
         << "  color_resolution - Color resolution for HDR output (default: 256, min: 224)\n"
         << "\nExamples:\n"
         << "  " << programName << " default output.ppm 64\n"
         << "  " << programName << " scenes/cornell_box.yaml render.ppm 128 800 600\n"
         << "  " << programName << " scenes/simple_test.yaml hdr_output.ppm 256 1024 768 512\n";
}

int main(int argc, char* argv[]) {
    if (argc < 4) {
        printUsage(argv[0]);
        return 1;
    }

    // Parse command line arguments
    string sceneFile = argv[1];
    string outputFile = argv[2];
    unsigned samples = stoul(argv[3]);
    
    // Optional parameters with defaults
    int width = (argc > 4) ? stoi(argv[4]) : 512;
    int height = (argc > 5) ? stoi(argv[5]) : 512;
    int colorResolution = (argc > 6) ? stoi(argv[6]) : 256;

    // Validate parameters
    if (samples == 0) {
        cerr << "Error: Number of samples must be greater than 0" << endl;
        return 1;
    }
    
    if (width <= 0 || height <= 0) {
        cerr << "Error: Width and height must be positive" << endl;
        return 1;
    }
    
    if (colorResolution < 224) {
        cerr << "Warning: Color resolution < 224 may cause artifacts. Using 224." << endl;
        colorResolution = 224;
    }

    try {
        cout << "=== PATH TRACER WITH REFLECTANCE PROPERTIES ===" << endl;
        cout << "Scene file: " << sceneFile << endl;
        cout << "Output file: " << outputFile << endl;
        cout << "Samples per pixel: " << samples << endl;
        cout << "Resolution: " << width << "x" << height << endl;
        cout << "Color resolution: " << colorResolution << endl;
        cout << endl;

        // Load scene from YAML file
        cout << "Loading scene..." << endl;
        Scene& scene = Scene::defaultScene();
        std::optional<PinholeCamera> cameraFromScene = std::nullopt;
        
        if (sceneFile != "default") {
            auto sceneOpt = Scene::fromYAML(sceneFile);
            if (sceneOpt) {
                scene = std::move(sceneOpt->first);
                cameraFromScene = sceneOpt->second;
            } else {
                cerr << "Error: Could not load scene from " << sceneFile << ". Using default scene." << endl;
            }
        }
        cout << "Scene loaded successfully" << endl;

        // Create camera (use from scene if available, otherwise default)
        PinholeCamera camera;
        if (cameraFromScene) {
            camera = *cameraFromScene;
            // Override resolution if specified
            if (argc > 4) {
                Point origin = camera.getOrigin();
                Direction forward = camera.getForward();
                int fov = static_cast<int>(camera.getFOV());
                camera = PinholeCamera(origin, fov, width, height, forward);
            }
            cout << "Using camera from scene file" << endl;
        } else {
            camera = PinholeCamera(Point(0, 0, -3), 50, width, height);
            cout << "Using default camera" << endl;
        }

        cout << "Camera: " << camera.toString() << endl;
        cout << endl;

        // Configure path tracing with reflectance properties
        RenderConfig config(RenderingAlgorithm::PATH_TRACING, samples, RenderingMode::PARALLEL);
        
        // Enable parallel rendering for better performance
        config.regionType = RegionType::RECTANGLE;
        config.regionSize = 16;
        config.numThreads = 4;
        
        cout << "Render configuration:" << endl;
        cout << "  Algorithm: PATH_TRACING" << endl;
        cout << "  Mode: PARALLEL (" << config.numThreads << " threads)" << endl;
        cout << "  Region type: " << toString(config.regionType) << endl;
        cout << "  Region size: " << config.regionSize << endl;
        cout << endl;

        // Start rendering
        cout << "Starting path tracing render..." << endl;
        auto startTime = chrono::high_resolution_clock::now();
        
        Image image = camera.render(scene, config);
        
        auto endTime = chrono::high_resolution_clock::now();
        auto duration = chrono::duration_cast<chrono::milliseconds>(endTime - startTime);
        
        cout << "Rendering completed in " << duration.count() / 1000.0 << " seconds" << endl;
        cout << "Image statistics:" << endl;
        cout << "  Resolution: " << image.width << "x" << image.height << endl;
        cout << "  Max color value: " << image.max() << endl;
        
        // Handle HDR output with color resolution
        if (image.max() > colorResolution) {
            cout << "HDR detected (max value: " << image.max() << " > " << colorResolution << ")" << endl;
            cout << "Consider applying tone mapping for display" << endl;
        }

        // Save image
        cout << "Saving image to " << outputFile << "..." << endl;
        
        string extension = outputFile.substr(outputFile.find_last_of('.') + 1);
        if (extension == "ppm") {
            image.writePPM(outputFile, colorResolution);
        } else if (extension == "bmp") {
            image.writeBMP(outputFile);
        } else {
            cerr << "Error: Unsupported output format '" << extension << "'" << endl;
            cerr << "Supported formats: .ppm, .bmp" << endl;
            return 1;
        }

        cout << "Path tracing completed successfully!" << endl;
        
        if (image.max() > 1.0f) {
            cout << endl;
            cout << "HDR output detected. To convert to LDR for display:" << endl;
            cout << "  ./build/tonemap " << outputFile << " ldr_output.ppm gamma 2.2" << endl;
            cout << "  ./build/tonemap " << outputFile << " ldr_output.ppm clamp 1.0" << endl;
        }

    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }

    return 0;
}
