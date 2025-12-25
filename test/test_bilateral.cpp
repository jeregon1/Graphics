#include <iostream>
#include <string>
#include <memory>
#include <cmath>

#include "../include/Image.hpp"
#include "../include/scene.hpp"
#include "../include/pinholeCamera.hpp"
#include "../include/object3D.hpp"
#include "../include/render_config.hpp"
#include "../include/geometry.hpp"

const std::string OUTPUT_DIR = "test_outputs/";

// Create a simple Cornell Box scene
Scene createCornellBox() {
    Scene scene(RGB(0, 0, 0));
    
    // Cornell Box dimensions
    const float boxSize = 2.0f;
    
    // Materials
    Material whiteMaterial(RGB(0.73, 0.73, 0.73), RGB(0, 0, 0));
    Material redMaterial(RGB(0.65, 0.05, 0.05), RGB(0, 0, 0));
    Material greenMaterial(RGB(0.12, 0.45, 0.15), RGB(0, 0, 0));
    
    // Walls
    scene.addObject(std::make_shared<Plane>(Direction(0, 1, 0), whiteMaterial, boxSize)); // Floor
    scene.addObject(std::make_shared<Plane>(Direction(0, -1, 0), whiteMaterial, boxSize)); // Ceiling
    scene.addObject(std::make_shared<Plane>(Direction(0, 0, -1), whiteMaterial, boxSize)); // Back wall
    scene.addObject(std::make_shared<Plane>(Direction(1, 0, 0), redMaterial, boxSize));   // Left wall (red)
    scene.addObject(std::make_shared<Plane>(Direction(-1, 0, 0), greenMaterial, boxSize)); // Right wall (green)
    
    // Add a sphere
    scene.addObject(std::make_shared<Sphere>(Point(-0.5, -1.3, 0.0), 0.7, whiteMaterial));
    
    // Light source (note: power/intensity is part of RGB in the constructor)
    scene.lights.push_back(std::make_shared<PointLight>(Point(0, 1.9, 0), RGB(15, 15, 15)));
    
    return scene;
}

void compareImages(const Image& img1, const Image& img2, const std::string& label) {
    if (img1.width != img2.width || img1.height != img2.height) {
        std::cout << "Images have different dimensions!" << std::endl;
        return;
    }
    
    double totalDiff = 0.0;
    double maxDiff = 0.0;
    
    for (size_t i = 0; i < img1.pixels.size(); i++) {
        RGB diff = img1.pixels[i] - img2.pixels[i];
        double pixelDiff = std::sqrt(diff.r * diff.r + diff.g * diff.g + diff.b * diff.b);
        totalDiff += pixelDiff;
        maxDiff = std::max(maxDiff, pixelDiff);
    }
    
    double avgDiff = totalDiff / img1.pixels.size();
    std::cout << "Comparison (" << label << "): Avg diff = " << avgDiff 
              << ", Max diff = " << maxDiff << std::endl;
}

int main() {
    const int width = 256;
    const int height = 256;
    
    std::cout << "Loading existing rendered image..." << std::endl;
    auto optImage = Image::readPPM("test_outputs/path_tracing/cornell_box.ppm");
    
    if (!optImage) {
        std::cerr << "Error: Could not load test_outputs/path_tracing/cornell_box.ppm" << std::endl;
        std::cerr << "Please render the scene first or check the file path." << std::endl;
        return 1;
    }
    
    Image original = std::move(*optImage);
    std::cout << "Image loaded successfully. Size: " << original.width << "x" << original.height << std::endl;
    
    // Save a copy as reference
    original.writePPM(OUTPUT_DIR + "gaussian_blur_original.ppm");
    std::cout << "Original image copy saved." << std::endl;
    
    // Test with different sigma values
    float sigmas[] = {0.5f, 2.0f, 5.0f};  // small, medium, large
    std::string sigma_labels[] = {"small", "medium", "large"};
    
    for (int i = 0; i < 3; i++) {
        std::cout << "Applying Gaussian blur with sigma = " << sigmas[i] << std::endl;
        Image dummy;  // Create temporary instance since gaussianBlur is not static
        Image filtered = dummy.gaussianBlur(original, sigmas[i]);
        
        std::string filename = OUTPUT_DIR + "gaussian_blur_" + sigma_labels[i] + ".ppm";
        filtered.writePPM(filename);
        std::cout << "Saved: " << filename << std::endl;
        
        compareImages(original, filtered, sigma_labels[i]);
    }
    
    std::cout << "\nTest completed. Images saved to " << OUTPUT_DIR << std::endl;
    std::cout << "Files generated:" << std::endl;
    std::cout << "  - gaussian_blur_original.ppm" << std::endl;
    std::cout << "  - gaussian_blur_small.ppm (sigma=0.5)" << std::endl;
    std::cout << "  - gaussian_blur_medium.ppm (sigma=2.0)" << std::endl;
    std::cout << "  - gaussian_blur_large.ppm (sigma=5.0)" << std::endl;
    
    return 0;
}