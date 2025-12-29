#include <iostream>
#include <string>
#include <memory>
#include <cmath>
#include <iomanip>

#include "../include/Image.hpp"
#include "../include/scene.hpp"
#include "../include/pinholeCamera.hpp"
#include "../include/object3D.hpp"
#include "../include/render_config.hpp"
#include "../include/geometry.hpp"

const std::string OUTPUT_DIR = "test_outputs/";

// Create a test scene with multiple objects for filter demonstration
Scene createTestScene() {
    Scene scene(RGB(0.1, 0.1, 0.1)); // Dark background
    
    // Materials
    Material redMaterial(RGB(0.8, 0.2, 0.2), RGB(0, 0, 0)); 
    Material greenMaterial(RGB(0.2, 0.8, 0.2), RGB(0, 0, 0)); 
    Material blueMaterial(RGB(0.2, 0.2, 0.8), RGB(0, 0, 0)); 
    Material greyMaterial(RGB(0.5, 0.5, 0.5), RGB(0, 0, 0));
    Material whiteMaterial(RGB(0.9, 0.9, 0.9), RGB(0, 0, 0));
    
    // Create multiple spheres at different positions to demonstrate filtering
    scene.addObject(std::make_shared<Sphere>(Point(-0.6, -0.2, 0.5), 0.3, redMaterial));
    scene.addObject(std::make_shared<Sphere>(Point(0.0, 0.1, 0.7), 0.25, greenMaterial));
    scene.addObject(std::make_shared<Sphere>(Point(0.6, -0.1, 0.6), 0.3, blueMaterial));
    scene.addObject(std::make_shared<Sphere>(Point(-0.2, 0.5, 1.0), 0.2, whiteMaterial));
    scene.addObject(std::make_shared<Sphere>(Point(0.3, 0.4, 0.8), 0.2, whiteMaterial));
    
    // Floor plane
    scene.addObject(std::make_shared<Plane>(Direction(0, 1, 0), greyMaterial, 1.5)); 
    
    // Lights - multiple light sources for richer illumination
    scene.addLight(std::make_shared<PointLight>(Point(0, 1.5, 0), RGB(4, 4, 4)));
    scene.addLight(std::make_shared<PointLight>(Point(-1, 0.8, -0.5), RGB(2, 1.5, 1.5)));
    scene.addLight(std::make_shared<PointLight>(Point(1, 0.8, -0.5), RGB(1.5, 1.5, 2)));
    
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
    const float sigma_space = 3.0f;
    
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
        
        std::string filename = OUTPUT_DIR + "bilateral_" + color_labels[i] + ".ppm";
        filtered.writePPM(filename);
        std::cout << "Saved: " << filename << std::endl;
        
        compareImages(original, filtered, color_labels[i]);
        std::cout << std::endl;
    }
    
    std::cout << "Bilateral filter test completed." << std::endl;
    std::cout << "Files generated:" << std::endl;
    std::cout << "  - bilateral_small.ppm (sigma_color=2.0, sigma_space=1.5)" << std::endl;
    std::cout << "  - bilateral_medium.ppm (sigma_color=5.0, sigma_space=1.5)" << std::endl;
    std::cout << "  - bilateral_large.ppm (sigma_color=10.0, sigma_space=1.5)" << std::endl;
    std::cout << "\nNote: Larger sigma_color values preserve more color differences" << std::endl;
    std::cout << "      and result in less smoothing across color boundaries." << std::endl;
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
    directImage.writePPM(OUTPUT_DIR + "lighting_direct.ppm");
    std::cout << "    Saved: lighting_direct.ppm" << std::endl;
    
    // Render indirect lighting only
    std::cout << "  - Rendering indirect lighting..." << std::endl;
    RenderConfig indirectConfig(RenderingAlgorithm::PATH_TRACING, 10, RenderingMode::SEQUENTIAL);
    indirectConfig.lightingDecomposition = LightingDecomposition::INDIRECT_ONLY;
    Image indirectImage = camera.render(scene, indirectConfig);
    indirectImage.writePPM(OUTPUT_DIR + "lighting_indirect_nofilter.ppm");
    std::cout << "    Saved: lighting_indirect_nofilter.ppm (unfiltered)" << std::endl;
    
    // Apply bilateral filter to indirect lighting
    std::cout << "  - Applying bilateral filter to indirect lighting..." << std::endl;
    Image dummy;
    // Using small parameters for subtle filtering (less blur)
    Image indirectFiltered = dummy.bilateralFilter(indirectImage, 3.0f, 0.2f);
    indirectFiltered.writePPM(OUTPUT_DIR + "lighting_indirect_filtered.ppm");
    std::cout << "    Saved: lighting_indirect_filtered.ppm (sigma_space=3.0, sigma_color=0.2)" << std::endl;
    
    // Combine direct + filtered indirect
    std::cout << "  - Combining direct + filtered indirect lighting..." << std::endl;
    std::vector<RGB> combined(directImage.pixels.size());
    for (size_t i = 0; i < combined.size(); i++) {
        combined[i] = directImage.pixels[i] + indirectFiltered.pixels[i];
    }
    Image combinedImage(directImage.width, directImage.height, combined);
    combinedImage.writePPM(OUTPUT_DIR + "lighting_combined_filtered.ppm");
    std::cout << "    Saved: lighting_combined_filtered.ppm (direct + filtered indirect)" << std::endl;
    
    // For comparison, also create unfiltered combined
    std::cout << "  - Creating unfiltered combined image for comparison..." << std::endl;
    std::vector<RGB> combinedUnfiltered(directImage.pixels.size());
    for (size_t i = 0; i < combinedUnfiltered.size(); i++) {
        combinedUnfiltered[i] = directImage.pixels[i] + indirectImage.pixels[i];
    }
    Image combinedUnfilteredImage(directImage.width, directImage.height, combinedUnfiltered);
    combinedUnfilteredImage.writePPM(OUTPUT_DIR + "lighting_combined_unfiltered.ppm");
    std::cout << "    Saved: lighting_combined_unfiltered.ppm (direct + unfiltered indirect)" << std::endl;
    
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
    std::cout << "  - lighting_direct.ppm (direct lighting only)" << std::endl;
    std::cout << "  - lighting_indirect_nofilter.ppm (indirect lighting, unfiltered)" << std::endl;
    std::cout << "  - lighting_indirect_filtered.ppm (indirect lighting, bilateral filter applied)" << std::endl;
    std::cout << "  - lighting_combined_unfiltered.ppm (direct + unfiltered indirect)" << std::endl;
    std::cout << "  - lighting_combined_filtered.ppm (direct + filtered indirect)" << std::endl;
}

int main(int argc, char* argv[]) {
    bool runDecompositionTest = false;
    
    // Check for command line argument
    if (argc > 1) {
        std::string arg = argv[1];
        if (arg == "--decompose" || arg == "-d") {
            runDecompositionTest = true;
        }
    }
    
    std::cout << "=== FILTER TESTS (BILATERAL & GAUSSIAN BLUR) ===" << std::endl;
    
    // Load Cornell Box scene from YAML
    std::cout << "Loading Cornell Box scene from YAML..." << std::endl;
    auto sceneResult = Scene::fromYAML("scenes/cornell_box.yaml");
    
    if (!sceneResult) {
        std::cerr << "Error: Could not load cornell_box.yaml" << std::endl;
        return 1;
    }
    
    Scene scene = std::move(sceneResult->first);
    PinholeCamera camera = sceneResult->second.has_value() ? sceneResult->second.value() 
                                                            : PinholeCamera(Point(0, 0, -2.5), 35, 512, 512);
    
    std::cout << "Cornell Box scene loaded successfully." << std::endl << std::endl;
    
    std::cout << "Rendering a single test image..." << std::endl << std::endl;
    
    // Render a single image with low samples to see filtering effects
    std::cout << "Generating test image with path tracing (low samples)..." << std::endl;
    RenderConfig config(RenderingAlgorithm::PATH_TRACING, 10, RenderingMode::SEQUENTIAL);
    
    std::cout << "Rendering with " << config.samplesPerPixel << " samples per pixel..." << std::endl;
    Image original = camera.render(scene, config);
    std::cout << "Image rendered. Size: " << original.width << "x" << original.height << std::endl;
    
    // Save the original for reference
    original.writePPM(OUTPUT_DIR + "filter_test_original.ppm");
    std::cout << "Original image saved as: filter_test_original.ppm\n" << std::endl;
    
    // Run both filter tests on the same image
    testBilateralFilter(original);
    testGaussianBlur(original);
    
    // Optional: Run lighting decomposition test
    if (runDecompositionTest) {
        testLightingDecomposition(scene, camera);
    }
    
    std::cout << "\n=== ALL TESTS COMPLETED ===" << std::endl;
    std::cout << "Images saved to " << OUTPUT_DIR << std::endl;
    std::cout << "Generated files:" << std::endl;
    std::cout << "  - filter_test_original.ppm (original noisy image)" << std::endl;
    std::cout << "  - bilateral_small.ppm, bilateral_medium.ppm, bilateral_large.ppm" << std::endl;
    std::cout << "  - gaussian_blur_small.ppm, gaussian_blur_medium.ppm, gaussian_blur_large.ppm" << std::endl;
    
    if (runDecompositionTest) {
        std::cout << "  - lighting_direct.ppm (direct lighting only)" << std::endl;
        std::cout << "  - lighting_indirect_nofilter.ppm (indirect lighting only, unfiltered)" << std::endl;
        std::cout << "  - lighting_indirect_filtered.ppm (indirect lighting, bilateral filtered)" << std::endl;
        std::cout << "  - lighting_combined_unfiltered.ppm (direct + unfiltered indirect)" << std::endl;
        std::cout << "  - lighting_combined_filtered.ppm (direct + filtered indirect)" << std::endl;
    } else {
        std::cout << "\nTip: Run with '--decompose' or '-d' flag to test lighting decomposition:" << std::endl;
        std::cout << "  ./build/test_bilateral --decompose" << std::endl;
    }
    
    return 0;
}