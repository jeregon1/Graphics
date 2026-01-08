#include <chrono>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include "../include/Image.hpp"
#include "../include/geometry.hpp"
#include "../include/pinholeCamera.hpp"
#include "../include/render_config.hpp"
#include "../include/scene.hpp"
#include "../include/toneMapping.hpp"

const std::string OUTPUT_DIR = "test_outputs/bilateral/";

struct RenderJob {
    std::string label;
    int samplesPerPixel;
    bool applyBilateral;
    float sigmaSpace;
    float sigmaColor;
};

struct RenderResult {
    Image image;
    double renderMs;
    double filterMs;
    bool filtered;
};

RenderResult renderScene(const Scene& scene,
                         const PinholeCamera& camera,
                         const RenderJob& job) {
    double renderMs = 0.0;
    double filterMs = 0.0;
    Image finalImage;

    if (job.applyBilateral) {
        // Direct lighting
        RenderConfig directCfg(RenderingAlgorithm::PATH_TRACING,
                               job.samplesPerPixel,
                               RenderingMode::SEQUENTIAL);
        directCfg.lightingDecomposition = LightingDecomposition::DIRECT_ONLY;
        auto d0 = std::chrono::steady_clock::now();
        Image direct = camera.render(scene, directCfg);
        auto d1 = std::chrono::steady_clock::now();
        renderMs += std::chrono::duration<double, std::milli>(d1 - d0).count();

        // Indirect lighting
        RenderConfig indirectCfg(RenderingAlgorithm::PATH_TRACING,
                                 job.samplesPerPixel,
                                 RenderingMode::SEQUENTIAL);
        indirectCfg.lightingDecomposition = LightingDecomposition::INDIRECT_ONLY;
        auto i0 = std::chrono::steady_clock::now();
        Image indirect = camera.render(scene, indirectCfg);
        auto i1 = std::chrono::steady_clock::now();
        renderMs += std::chrono::duration<double, std::milli>(i1 - i0).count();

        // Filter only indirect component
        auto f0 = std::chrono::steady_clock::now();
        Image dummy;
        Image indirectFiltered = dummy.bilateralFilter(indirect, job.sigmaSpace, job.sigmaColor);
        auto f1 = std::chrono::steady_clock::now();
        filterMs = std::chrono::duration<double, std::milli>(f1 - f0).count();

        // Combine direct + filtered indirect
        std::vector<RGB> combined(direct.width * direct.height);
        for (size_t k = 0; k < combined.size(); ++k) {
            combined[k] = direct.pixels[k] + indirectFiltered.pixels[k];
        }
        finalImage = Image(direct.width, direct.height, combined);

    } else {
        RenderConfig cfg(RenderingAlgorithm::PATH_TRACING,
                         job.samplesPerPixel,
                         RenderingMode::SEQUENTIAL);
        auto t0 = std::chrono::steady_clock::now();
        finalImage = camera.render(scene, cfg);
        auto t1 = std::chrono::steady_clock::now();
        renderMs = std::chrono::duration<double, std::milli>(t1 - t0).count();
    }

    // Tone mapping for visibility
    ToneMapping::gamma(finalImage, 2.2f);

    return RenderResult{std::move(finalImage), renderMs, filterMs, job.applyBilateral};
}

void runComparison() {
    // Ensure output directory exists
    std::filesystem::create_directories(OUTPUT_DIR);

    auto sceneResult = Scene::fromYAML("scenes/cornell_box.yaml");
    if (!sceneResult) {
        std::cerr << "Error: Could not load scenes/cornell_box.yaml" << std::endl;
        return;
    }

    Scene scene = std::move(sceneResult->first);
    PinholeCamera camera = sceneResult->second.has_value()
                               ? sceneResult->second.value()
                               : PinholeCamera(Point(0, 0, -2.5), 35, 512, 512);

    std::vector<RenderJob> jobs = {
        // Más separación en SPP para que el cambio de ruido sea perceptible
        {"low_spp_filtered", 8, true, 2.0f, 0.10f},     // Filtro algo más fuerte para suprimir ruido
        {"mid_spp_filtered", 32, true, 1.5f, 0.06f},   // Más SPP y filtro más suave para conservar detalle
        {"high_spp_no_filter", 128, false, 0.0f, 0.0f}, // Alto muestreo sin filtro
    };

    std::vector<RenderResult> results;
    results.reserve(jobs.size());

    std::cout << "=== Bilateral vs High Sampling Comparison ===" << std::endl;
    std::cout << "Scene: Cornell Box" << std::endl;
    std::cout << std::endl;

    // Warm-up to build acceleration structures / caches before timing
    {
        RenderConfig warmCfg(RenderingAlgorithm::PATH_TRACING, 1, RenderingMode::SEQUENTIAL);
        warmCfg.lightingDecomposition = LightingDecomposition::NONE;
        warmCfg.toneMapping = ToneMappingType::NONE;
        camera.render(scene, warmCfg);
    }

    for (const auto& job : jobs) {
        std::cout << "Rendering job: " << job.label << std::endl;
        std::cout << "  samplesPerPixel: " << job.samplesPerPixel << std::endl;
        if (job.applyBilateral) {
            std::cout << "  bilateral: sigma_space=" << job.sigmaSpace
                      << ", sigma_color=" << job.sigmaColor << std::endl;
        } else {
            std::cout << "  bilateral: off" << std::endl;
        }

        RenderResult result = renderScene(scene, camera, job);

        std::string filename = OUTPUT_DIR + "bilateral_time_" + job.label + ".ppm";
        result.image.writePPM(filename);

        std::cout << "  Render time: " << std::fixed << std::setprecision(2)
                  << result.renderMs << " ms" << std::endl;
        if (result.filtered) {
            std::cout << "  Bilateral time: " << std::fixed << std::setprecision(2)
                      << result.filterMs << " ms" << std::endl;
        }
        double totalMs = result.renderMs + result.filterMs;
        std::cout << "  Total time: " << std::fixed << std::setprecision(2)
                  << totalMs << " ms" << std::endl;
        std::cout << "  Saved: " << filename << std::endl << std::endl;

        results.push_back(std::move(result));
    }

    // Summary comparison
    std::cout << "=== Timing Summary ===" << std::endl;
    std::cout << std::left << std::setw(22) << "Job"
              << std::right << std::setw(12) << "SPP"
              << std::setw(14) << "Render ms"
              << std::setw(14) << "Filter ms"
              << std::setw(14) << "Total ms" << std::endl;

    for (size_t i = 0; i < jobs.size(); ++i) {
        const auto& job = jobs[i];
        const auto& r = results[i];
        double total = r.renderMs + r.filterMs;
        std::cout << std::left << std::setw(22) << job.label
                  << std::right << std::setw(12) << job.samplesPerPixel
                  << std::setw(14) << std::fixed << std::setprecision(2) << r.renderMs
                  << std::setw(14) << (job.applyBilateral ? r.filterMs : 0.0)
                  << std::setw(14) << total
                  << std::endl;
    }
}

int main() {
    runComparison();
    return 0;
}