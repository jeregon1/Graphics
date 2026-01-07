#include <iostream>
#include <string>
#include <memory>
#include <chrono>
#include <iomanip>

#include "../include/Image.hpp"
#include "../include/scene.hpp"
#include "../include/pinholeCamera.hpp"
#include "../include/object3D.hpp"
#include "../include/render_config.hpp"
#include "../include/geometry.hpp"
#include "../include/toneMapping.hpp"

const std::string OUTPUT_DIR = "test_outputs/";

// Create Cornell Box with point light and ceiling
Scene createCornellBoxPointLight() {
    Scene scene(RGB(0, 0, 0));
    
    // Cornell Box dimensions
    const float boxSize = 2.0f;
    
    // Materials
    Material whiteMaterial(RGB(0.73, 0.73, 0.73), RGB(0, 0, 0));
    Material redMaterial(RGB(0.65, 0.05, 0.05), RGB(0, 0, 0));
    Material greenMaterial(RGB(0.12, 0.45, 0.15), RGB(0, 0, 0));
    
    // Walls including ceiling
    scene.addObject(std::make_shared<Plane>(Direction(0, 1, 0), whiteMaterial, boxSize));  // Floor
    scene.addObject(std::make_shared<Plane>(Direction(0, -1, 0), whiteMaterial, boxSize)); // Ceiling
    scene.addObject(std::make_shared<Plane>(Direction(0, 0, -1), whiteMaterial, boxSize)); // Back wall
    scene.addObject(std::make_shared<Plane>(Direction(1, 0, 0), redMaterial, boxSize));    // Left wall (red)
    scene.addObject(std::make_shared<Plane>(Direction(-1, 0, 0), greenMaterial, boxSize)); // Right wall (green)
    
    // Add a sphere
    scene.addObject(std::make_shared<Sphere>(Point(-0.5, -1.3, 0.0), 0.7, whiteMaterial));
    
    // Point light source at ceiling
    scene.lights.push_back(std::make_shared<PointLight>(Point(0, 1.9, 0), RGB(15, 15, 15)));
    
    return scene;
}

// Create Cornell Box with area light (emissive ceiling)
Scene createCornellBoxAreaLight() {
    Scene scene(RGB(0, 0, 0));
    
    // Cornell Box dimensions
    const float boxSize = 2.0f;
    
    // Materials
    Material whiteMaterial(RGB(0.73, 0.73, 0.73), RGB(0, 0, 0));
    Material redMaterial(RGB(0.65, 0.05, 0.05), RGB(0, 0, 0));
    Material greenMaterial(RGB(0.12, 0.45, 0.15), RGB(0, 0, 0));
    
    // Emissive material for ceiling (area light)
    Material emissiveMaterial(RGB(0.73, 0.73, 0.73), RGB(15, 15, 15)); // emission = RGB(15, 15, 15)
    
    // Walls with emissive ceiling
    scene.addObject(std::make_shared<Plane>(Direction(0, 1, 0), whiteMaterial, boxSize));     // Floor
    scene.addObject(std::make_shared<Plane>(Direction(0, -1, 0), emissiveMaterial, boxSize)); // Ceiling (emissive)
    scene.addObject(std::make_shared<Plane>(Direction(0, 0, -1), whiteMaterial, boxSize));    // Back wall
    scene.addObject(std::make_shared<Plane>(Direction(1, 0, 0), redMaterial, boxSize));       // Left wall (red)
    scene.addObject(std::make_shared<Plane>(Direction(-1, 0, 0), greenMaterial, boxSize));    // Right wall (green)
    
    // Add a sphere
    scene.addObject(std::make_shared<Sphere>(Point(-0.5, -1.3, 0.0), 0.7, whiteMaterial));
    
    // No point lights - using emissive ceiling instead
    
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

void testBilateralFilter(const Image& original) {
    std::cout << "\n========================================" << std::endl;
    std::cout << "=== BILATERAL FILTER TEST ===" << std::endl;
    std::cout << "========================================\n" << std::endl;
    
    // Bilateral filter parameters
    // σd (sigma_space) - spatial parameter, controls spatial spread
    const float sigma_space = 2.0f;
    
    // σr (sigma_color) - range parameter, controls color sensitivity
    // The image values are in linear color space (not clamped to [0-255])
    // We need larger sigma_color values to properly filter
    float sigma_colors[] = {0.02f, 0.08f, 0.2f};  // small, medium, large
    std::string color_labels[] = {"small", "medium", "large"};
    
    std::cout << "\nFixed spatial parameter (sigma_space): " << sigma_space << std::endl;
    std::cout << "  (radius = ceil(3 * " << sigma_space << ") = " << static_cast<int>(std::ceil(3.0f * sigma_space)) << " pixels)" << std::endl;
    std::cout << "Varying color parameter (sigma_color): small, medium, large" << std::endl;
    std::cout << "Note: Working in linear color space (not 8-bit)\n" << std::endl;
    
    for (int i = 0; i < 3; i++) {
        std::cout << "Applying bilateral filter with sigma_color = " << sigma_colors[i] 
                  << " (label: " << color_labels[i] << ")" << std::endl;
        
        Image dummy;  // Create temporary instance since bilateralFilter is not static
        Image filtered = dummy.bilateralFilter(original, sigma_space, sigma_colors[i]);
        
        // Apply gamma correction for better visibility
        ToneMapping::gamma(filtered, 2.2f);
        
        std::string filename = OUTPUT_DIR + "bilateral_" + color_labels[i] + ".ppm";
        filtered.writePPM(filename);
        std::cout << "Saved: " << filename << std::endl;
        
        compareImages(original, filtered, color_labels[i]);
        std::cout << std::endl;
    }
    
    std::cout << "Bilateral filter test completed." << std::endl;
    std::cout << "Files generated:" << std::endl;
    std::cout << "  - bilateral_small.ppm (sigma_color=0.02, sigma_space=2.0)" << std::endl;
    std::cout << "  - bilateral_medium.ppm (sigma_color=0.08, sigma_space=2.0)" << std::endl;
    std::cout << "  - bilateral_large.ppm (sigma_color=0.2, sigma_space=2.0)" << std::endl;
    std::cout << "\nNote: Larger sigma_color values smooth more across color differences" << std::endl;
    std::cout << "      while smaller values preserve edges more aggressively." << std::endl;
}

void testGaussianBlur(const Image& original) {
    std::cout << "\n========================================" << std::endl;
    std::cout << "=== GAUSSIAN BLUR TEST ===" << std::endl;
    std::cout << "========================================\n" << std::endl;
    
    // Test with different sigma values
    float sigmas[] = {0.5f, 2.0f, 5.0f};  // small, medium, large
    std::string sigma_labels[] = {"small", "medium", "large"};
    
    std::cout << "Testing Gaussian blur with varying sigma values\n" << std::endl;
    
    for (int i = 0; i < 3; i++) {
        std::cout << "Applying Gaussian blur with sigma = " << sigmas[i] << std::endl;
        Image dummy;  // Create temporary instance since gaussianBlur is not static
        Image filtered = dummy.gaussianBlur(original, sigmas[i]);
        
        // Apply gamma correction for better visibility
        ToneMapping::gamma(filtered, 2.2f);
        
        std::string filename = OUTPUT_DIR + "gaussian_blur_" + sigma_labels[i] + ".ppm";
        filtered.writePPM(filename);
        std::cout << "Saved: " << filename << std::endl;
        
        compareImages(original, filtered, sigma_labels[i]);
        std::cout << std::endl;
    }
    
    std::cout << "Gaussian blur test completed." << std::endl;
    std::cout << "Files generated:" << std::endl;
    std::cout << "  - gaussian_blur_small.ppm (sigma=0.5)" << std::endl;
    std::cout << "  - gaussian_blur_medium.ppm (sigma=2.0)" << std::endl;
    std::cout << "  - gaussian_blur_large.ppm (sigma=5.0)" << std::endl;
}

void testLightingDecomposition(const Scene& scene, const PinholeCamera& camera) {
    std::cout << "\n========================================" << std::endl;
    std::cout << "=== LIGHTING DECOMPOSITION TEST ===" << std::endl;
    std::cout << "========================================\n" << std::endl;
    
    std::cout << "Rendering with lighting decomposition and bilateral filtering..." << std::endl;
    
    // Render direct lighting only
    std::cout << "  - Rendering direct lighting..." << std::endl;
    RenderConfig directConfig(RenderingAlgorithm::PATH_TRACING, 10, RenderingMode::SEQUENTIAL);
    directConfig.lightingDecomposition = LightingDecomposition::DIRECT_ONLY;
    Image directImage = camera.render(scene, directConfig);
    Image directImageTM = directImage;  // Copy for tone mapping
    ToneMapping::gamma(directImageTM, 2.2f);
    directImageTM.writePPM(OUTPUT_DIR + "lighting_direct.ppm");
    std::cout << "    Saved: lighting_direct.ppm (tone-mapped)" << std::endl;
    
    // Render indirect lighting only
    std::cout << "  - Rendering indirect lighting..." << std::endl;
    RenderConfig indirectConfig(RenderingAlgorithm::PATH_TRACING, 10, RenderingMode::SEQUENTIAL);
    indirectConfig.lightingDecomposition = LightingDecomposition::INDIRECT_ONLY;
    Image indirectImage = camera.render(scene, indirectConfig);
    Image indirectImageTM = indirectImage;  // Copy for tone mapping
    ToneMapping::gamma(indirectImageTM, 2.2f);
    indirectImageTM.writePPM(OUTPUT_DIR + "lighting_indirect_nofilter.ppm");
    std::cout << "    Saved: lighting_indirect_nofilter.ppm (unfiltered, tone-mapped)" << std::endl;
    
    // Apply bilateral filter to indirect lighting
    std::cout << "  - Applying bilateral filter to indirect lighting..." << std::endl;
    Image dummy;
    // Using small parameters for subtle filtering (less blur)
    Image indirectFiltered = dummy.bilateralFilter(indirectImage, 3.0f, 0.2f);
    Image indirectFilteredTM = indirectFiltered;  // Copy for tone mapping
    ToneMapping::gamma(indirectFilteredTM, 2.2f);
    indirectFilteredTM.writePPM(OUTPUT_DIR + "lighting_indirect_filtered.ppm");
    std::cout << "    Saved: lighting_indirect_filtered.ppm (sigma_space=3.0, sigma_color=0.2, tone-mapped)" << std::endl;
    
    // Combine direct + filtered indirect
    std::cout << "  - Combining direct + filtered indirect lighting..." << std::endl;
    std::vector<RGB> combined(directImage.pixels.size());
    for (size_t i = 0; i < combined.size(); i++) {
        combined[i] = directImage.pixels[i] + indirectFiltered.pixels[i];
    }
    Image combinedImage(directImage.width, directImage.height, combined);
    ToneMapping::gamma(combinedImage, 2.2f);
    combinedImage.writePPM(OUTPUT_DIR + "lighting_combined_filtered.ppm");
    std::cout << "    Saved: lighting_combined_filtered.ppm (direct + filtered indirect, tone-mapped)" << std::endl;
    
    // For comparison, also create unfiltered combined
    std::cout << "  - Creating unfiltered combined image for comparison..." << std::endl;
    std::vector<RGB> combinedUnfiltered(directImage.pixels.size());
    for (size_t i = 0; i < combinedUnfiltered.size(); i++) {
        combinedUnfiltered[i] = directImage.pixels[i] + indirectImage.pixels[i];
    }
    Image combinedUnfilteredImage(directImage.width, directImage.height, combinedUnfiltered);
    ToneMapping::gamma(combinedUnfilteredImage, 2.2f);
    combinedUnfilteredImage.writePPM(OUTPUT_DIR + "lighting_combined_unfiltered.ppm");
    std::cout << "    Saved: lighting_combined_unfiltered.ppm (direct + unfiltered indirect, tone-mapped)" << std::endl;
    
    // Compare filtering effect
    std::cout << "\nFiltering effect analysis:" << std::endl;
    double totalDiffUnfiltered = 0.0;
    double totalDiffFiltered = 0.0;
    double maxDiffUnfiltered = 0.0;
    double maxDiffFiltered = 0.0;
    
    for (size_t i = 0; i < directImage.pixels.size(); i++) {
        // Difference between unfiltered and direct (noise)
        RGB diffUnfilt = indirectImage.pixels[i] - directImage.pixels[i];
        double pixelDiffUnfilt = std::sqrt(diffUnfilt.r * diffUnfilt.r + diffUnfilt.g * diffUnfilt.g + diffUnfilt.b * diffUnfilt.b);
        totalDiffUnfiltered += pixelDiffUnfilt;
        maxDiffUnfiltered = std::max(maxDiffUnfiltered, pixelDiffUnfilt);
        
        // Difference between filtered and direct (remaining noise)
        RGB diffFilt = indirectFiltered.pixels[i] - directImage.pixels[i];
        double pixelDiffFilt = std::sqrt(diffFilt.r * diffFilt.r + diffFilt.g * diffFilt.g + diffFilt.b * diffFilt.b);
        totalDiffFiltered += pixelDiffFilt;
        maxDiffFiltered = std::max(maxDiffFiltered, pixelDiffFilt);
    }
    
    double avgDiffUnfiltered = totalDiffUnfiltered / directImage.pixels.size();
    double avgDiffFiltered = totalDiffFiltered / directImage.pixels.size();
    double noiseReduction = ((avgDiffUnfiltered - avgDiffFiltered) / avgDiffUnfiltered) * 100.0;
    
    std::cout << "Unfiltered indirect vs direct:" << std::endl;
    std::cout << "  - Average difference: " << avgDiffUnfiltered << std::endl;
    std::cout << "  - Maximum difference: " << maxDiffUnfiltered << std::endl;
    
    std::cout << "Filtered indirect vs direct:" << std::endl;
    std::cout << "  - Average difference: " << avgDiffFiltered << std::endl;
    std::cout << "  - Maximum difference: " << maxDiffFiltered << std::endl;
    
    std::cout << "Noise reduction: " << std::fixed << std::setprecision(2) << noiseReduction << "%" << std::endl;
    
    std::cout << "\nLighting decomposition test completed." << std::endl;
    std::cout << "Files generated:" << std::endl;
    std::cout << "  - lighting_direct.ppm (direct lighting only, tone-mapped)" << std::endl;
    std::cout << "  - lighting_indirect_nofilter.ppm (indirect lighting, unfiltered, tone-mapped)" << std::endl;
    std::cout << "  - lighting_indirect_filtered.ppm (indirect lighting, bilateral filter applied, tone-mapped)" << std::endl;
    std::cout << "  - lighting_combined_unfiltered.ppm (direct + unfiltered indirect, tone-mapped)" << std::endl;
    std::cout << "  - lighting_combined_filtered.ppm (direct + filtered indirect, tone-mapped)" << std::endl;
}


int main() {
    std::cout << "=== LIGHTING COMPARISON TEST ===" << std::endl;
    std::cout << "Comparing Point Light vs Area Light convergence\n" << std::endl;
    
    const int width = 512;
    const int height = 512;
    const int samples = 100;
    
    // Create camera
    PinholeCamera camera(Point(0, 0, -4), 50, width, height);
    
    // ========== POINT LIGHT RENDERING ==========
    std::cout << "========================================" << std::endl;
    std::cout << "Rendering with POINT LIGHT" << std::endl;
    std::cout << "========================================" << std::endl;
    
    Scene scenePointLight = createCornellBoxPointLight();
    std::cout << "Scene created with point light at ceiling" << std::endl;
    std::cout << "Rendering with " << samples << " samples per pixel..." << std::endl;
    
    auto startPointLight = std::chrono::high_resolution_clock::now();
    
    RenderConfig configPointLight;
    configPointLight.algorithm = RenderingAlgorithm::PATH_TRACING;
    configPointLight.samplesPerPixel = samples;
    
    Image imagePointLight = camera.render(scenePointLight, configPointLight);
    
    auto endPointLight = std::chrono::high_resolution_clock::now();
    auto durationPointLight = std::chrono::duration_cast<std::chrono::milliseconds>(endPointLight - startPointLight);
    
    std::cout << "Rendering completed in " << std::fixed << std::setprecision(2) 
              << durationPointLight.count() / 1000.0 << " seconds" << std::endl;
    
    // Apply tone mapping and save
    ToneMapping::gamma(imagePointLight, 2.2f);
    imagePointLight.writePPM(OUTPUT_DIR + "point_light_cornell.ppm");
    std::cout << "Saved: point_light_cornell.ppm\n" << std::endl;
    
    // ========== AREA LIGHT RENDERING ==========
    std::cout << "========================================" << std::endl;
    std::cout << "Rendering with AREA LIGHT (Emissive Ceiling)" << std::endl;
    std::cout << "========================================" << std::endl;
    
    Scene sceneAreaLight = createCornellBoxAreaLight();
    std::cout << "Scene created with emissive ceiling (area light)" << std::endl;
    std::cout << "Rendering with " << samples << " samples per pixel..." << std::endl;
    
    auto startAreaLight = std::chrono::high_resolution_clock::now();
    
    RenderConfig configAreaLight;
    configAreaLight.algorithm = RenderingAlgorithm::PATH_TRACING;
    configAreaLight.samplesPerPixel = samples;
    
    Image imageAreaLight = camera.render(sceneAreaLight, configAreaLight);
    
    auto endAreaLight = std::chrono::high_resolution_clock::now();
    auto durationAreaLight = std::chrono::duration_cast<std::chrono::milliseconds>(endAreaLight - startAreaLight);
    
    std::cout << "Rendering completed in " << std::fixed << std::setprecision(2) 
              << durationAreaLight.count() / 1000.0 << " seconds" << std::endl;
    
    // Apply tone mapping and save
    ToneMapping::gamma(imageAreaLight, 2.2f);
    imageAreaLight.writePPM(OUTPUT_DIR + "area_light_cornell.ppm");
    std::cout << "Saved: area_light_cornell.ppm\n" << std::endl;
    
    // ========== COMPARISON RESULTS ==========
    std::cout << "========================================" << std::endl;
    std::cout << "CONVERGENCE COMPARISON" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Configuration: " << width << "x" << height << " @ " << samples << " spp" << std::endl;
    std::cout << std::endl;
    std::cout << "Point Light:" << std::endl;
    std::cout << "  Time: " << std::fixed << std::setprecision(2) 
              << durationPointLight.count() / 1000.0 << " seconds" << std::endl;
    std::cout << "  Speed: " << std::fixed << std::setprecision(1)
              << (width * height * samples) / (durationPointLight.count() / 1000.0) 
              << " samples/second" << std::endl;
    std::cout << std::endl;
    std::cout << "Area Light (Emissive Ceiling):" << std::endl;
    std::cout << "  Time: " << std::fixed << std::setprecision(2) 
              << durationAreaLight.count() / 1000.0 << " seconds" << std::endl;
    std::cout << "  Speed: " << std::fixed << std::setprecision(1)
              << (width * height * samples) / (durationAreaLight.count() / 1000.0) 
              << " samples/second" << std::endl;
    std::cout << std::endl;
    
    double speedRatio = static_cast<double>(durationPointLight.count()) / durationAreaLight.count();
    if (speedRatio > 1.0) {
        std::cout << "Area light was " << std::fixed << std::setprecision(2) 
                  << speedRatio << "x FASTER than point light" << std::endl;
    } else {
        std::cout << "Point light was " << std::fixed << std::setprecision(2) 
                  << (1.0 / speedRatio) << "x FASTER than area light" << std::endl;
    }
    
    std::cout << "\n=== TEST COMPLETED ===" << std::endl;
    std::cout << "Images saved to " << OUTPUT_DIR << std::endl;
    std::cout << "Generated files:" << std::endl;
    std::cout << "  - point_light_cornell.ppm" << std::endl;
    std::cout << "  - area_light_cornell.ppm" << std::endl;
    
    return 0;
}