#include <chrono>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include "../include/Image.hpp"
#include "../include/pinholeCamera.hpp"
#include "../include/render_config.hpp"
#include "../include/scene.hpp"
#include "../include/toneMapping.hpp"

const std::string OUTPUT_DIR = "test_outputs/bilateral/";

Image applyBilateralFilter(const Image& src, float sigmaSpace, float sigmaColor) {
    Image dummy;
    return dummy.bilateralFilter(src, sigmaSpace, sigmaColor);
}

void saveToned(const std::string& path, const Image& img) {
    Image tmp = img;
    ToneMapping::gamma(tmp, 2.2f);
    tmp.writePPM(path);
    std::cout << "  Saved: " << path << std::endl;
}

int main() {
    std::filesystem::create_directories(OUTPUT_DIR);

    std::cout << "=== Bilateral Filter Separation Test ===" << std::endl;

    auto sceneResult = Scene::fromYAML("scenes/cornell_box.yaml");
    if (!sceneResult) {
        std::cerr << "Error: Could not load scenes/cornell_box.yaml" << std::endl;
        return 1;
    }

    Scene scene = std::move(sceneResult->first);
    PinholeCamera camera = sceneResult->second.has_value()
                               ? sceneResult->second.value()
                               : PinholeCamera(Point(0, 0, -2.5), 35, 512, 512);

    const int highSPP = 64;
    const int lowSpp = 16;
    const float sigmaDirect = 1.0f;
    const float sigmaIndirect = 2.0f;
    const float sigmaColor = 0.08f;

    // ============================================================
    // STEP 1: RENDER DIRECT LIGHTING
    // ============================================================
    std::cout << "\n[1/6] Rendering DIRECT lighting (" << lowSpp << " spp)..." << std::endl;
    auto t_direct_start = std::chrono::steady_clock::now();
    RenderConfig cfgDirect(RenderingAlgorithm::PATH_TRACING, lowSpp, RenderingMode::PARALLEL);
    cfgDirect.lightingDecomposition = LightingDecomposition::DIRECT_ONLY;
    Image direct = camera.render(scene, cfgDirect);
    auto t_direct_end = std::chrono::steady_clock::now();
    double t_direct_ms = std::chrono::duration<double, std::milli>(t_direct_end - t_direct_start).count();
    saveToned(OUTPUT_DIR + "01_direct.ppm", direct);

    // ============================================================
    // STEP 2: RENDER INDIRECT LIGHTING
    // ============================================================
    std::cout << "\n[2/6] Rendering INDIRECT lighting (" << highSPP << " spp)..." << std::endl;
    auto t_indirect_start = std::chrono::steady_clock::now();
    RenderConfig cfgIndirect(RenderingAlgorithm::PATH_TRACING, highSPP, RenderingMode::PARALLEL);
    cfgIndirect.lightingDecomposition = LightingDecomposition::INDIRECT_ONLY;
    Image indirect = camera.render(scene, cfgIndirect);
    auto t_indirect_end = std::chrono::steady_clock::now();
    double t_indirect_ms = std::chrono::duration<double, std::milli>(t_indirect_end - t_indirect_start).count();
    saveToned(OUTPUT_DIR + "02_indirect.ppm", indirect);

    // ============================================================
    // STEP 3: RENDER COMBINED LIGHTING
    // ============================================================
    std::cout << "\n[3/6] Rendering COMBINED lighting (" << highSPP << " spp)..." << std::endl;
    auto t_combined_start = std::chrono::steady_clock::now();
    RenderConfig cfgCombined(RenderingAlgorithm::PATH_TRACING, highSPP, RenderingMode::PARALLEL);
    Image combined = camera.render(scene, cfgCombined);
    auto t_combined_end = std::chrono::steady_clock::now();
    double t_combined_ms = std::chrono::duration<double, std::milli>(t_combined_end - t_combined_start).count();
    saveToned(OUTPUT_DIR + "03_combined.ppm", combined);

    // ============================================================
    // STEP 4: FILTER INDIRECT LIGHTING ONLY
    // ============================================================
    std::cout << "\n[4/6] Applying bilateral filter to INDIRECT lighting (sigma=" << sigmaIndirect << ")..." << std::endl;
    auto t_filt_indirect_start = std::chrono::steady_clock::now();
    Image indirectFiltered = applyBilateralFilter(indirect, sigmaIndirect, sigmaColor);
    auto t_filt_indirect_end = std::chrono::steady_clock::now();
    double t_filt_indirect_ms = std::chrono::duration<double, std::milli>(t_filt_indirect_end - t_filt_indirect_start).count();
    saveToned(OUTPUT_DIR + "04_indirect_filtered.ppm", indirectFiltered);

    // ============================================================
    // STEP 5: COMBINE FILTERED INDIRECT WITH RAW DIRECT
    // ============================================================
    std::cout << "\n[5/6] Combining filtered indirect with raw direct..." << std::endl;
    Image separationCombined(direct.width, direct.height);
    for (size_t i = 0; i < direct.pixels.size(); ++i) {
        separationCombined.pixels[i] = direct.pixels[i] + indirectFiltered.pixels[i];
    }
    saveToned(OUTPUT_DIR + "05_separated_combined.ppm", separationCombined);

    // Also filter the combined image for comparison
    auto t_filt_combined_start = std::chrono::steady_clock::now();
    Image combinedFiltered = applyBilateralFilter(combined, (sigmaDirect + sigmaIndirect) / 2.0f, sigmaColor);
    auto t_filt_combined_end = std::chrono::steady_clock::now();
    double t_filt_combined_ms = std::chrono::duration<double, std::milli>(t_filt_combined_end - t_filt_combined_start).count();
    saveToned(OUTPUT_DIR + "06_combined_filtered.ppm", combinedFiltered);

    // ============================================================
    // RESULTS
    // ============================================================
    std::cout << "\n" << std::string(70, '=') << std::endl;
    std::cout << "=== RENDERING TIMES ===" << std::endl;
    std::cout << std::string(70, '=') << std::endl;
    std::cout << "  Direct lighting:        " << std::fixed << std::setprecision(1) << t_direct_ms << " ms" << std::endl;
    std::cout << "  Indirect lighting:      " << std::fixed << std::setprecision(1) << t_indirect_ms << " ms" << std::endl;
    std::cout << "  Combined lighting:      " << std::fixed << std::setprecision(1) << t_combined_ms << " ms" << std::endl;
    std::cout << "  Total rendering:        " << std::fixed << std::setprecision(1) 
              << (t_direct_ms + t_indirect_ms + t_combined_ms) << " ms" << std::endl;

    std::cout << "\n" << std::string(70, '=') << std::endl;
    std::cout << "=== FILTERING TIMES ===" << std::endl;
    std::cout << std::string(70, '=') << std::endl;
    std::cout << "  Indirect filtered:      " << std::fixed << std::setprecision(1) << t_filt_indirect_ms << " ms" << std::endl;
    std::cout << "  Combined filtered:      " << std::fixed << std::setprecision(1) << t_filt_combined_ms << " ms" << std::endl;
    std::cout << "  Total filtering:        " << std::fixed << std::setprecision(1) 
              << (t_filt_indirect_ms + t_filt_combined_ms) << " ms" << std::endl;

    std::cout << "\n" << std::string(70, '=') << std::endl;
    std::cout << "=== IMAGE FILES GENERATED ===" << std::endl;
    std::cout << std::string(70, '=') << std::endl;
    std::cout << "  01_direct.ppm                 - Raw direct lighting" << std::endl;
    std::cout << "  02_indirect.ppm               - Raw indirect lighting" << std::endl;
    std::cout << "  03_combined.ppm               - Raw combined lighting (direct+indirect)" << std::endl;
    std::cout << "  04_indirect_filtered.ppm      - Indirect with bilateral filter (sigma=" << sigmaIndirect << ")" << std::endl;
    std::cout << "  05_separated_combined.ppm     - Combined from filtered indirect + raw direct" << std::endl;
    std::cout << "  06_combined_filtered.ppm      - Combined with bilateral filter (sigma=" << (sigmaDirect+sigmaIndirect)/2.0f << ")" << std::endl;
    std::cout << std::string(70, '=') << std::endl;
    std::cout << "\n[ADAPTIVE SAMPLING INFO]" << std::endl;
    std::cout << "  Direct lighting:   " << lowSpp << " samples/pixel (fast, low noise)" << std::endl;
    std::cout << "  Indirect lighting: " << highSPP << " samples/pixel (slow, high noise reduction)" << std::endl;
    std::cout << "  Combined:          " << highSPP << " samples/pixel (reference for comparison)" << std::endl;
    std::cout << std::string(70, '=') << std::endl;

    return 0;
}