#include <chrono>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <vector>

#include "../include/Image.hpp"
#include "../include/kernel.hpp"
#include "../include/pinholeCamera.hpp"
#include "../include/render_config.hpp"
#include "../include/scene.hpp"
#include "../include/toneMapping.hpp"

const std::string OUTPUT_DIR = "test_outputs/photon_k_scaling/";

void saveToned(const std::string& path, const Image& img) {
    Image tmp = img;
    ToneMapping::gamma(tmp, 2.2f);
    tmp.writePPM(path);
    std::cout << "    Saved: " << path << std::endl;
}

int main() {
    std::filesystem::create_directories(OUTPUT_DIR);

    std::cout << "\n" << std::string(80, '=') << std::endl;
    std::cout << "=== PHOTON MAPPING K-NEIGHBOR SCALING TEST ===" << std::endl;
    std::cout << std::string(80, '=') << std::endl;

    // Load scene
    auto sceneResult = Scene::fromYAML("scenes/cornell_box.yaml");
    if (!sceneResult) {
        std::cerr << "Error: Could not load scenes/cornell_box.yaml" << std::endl;
        return 1;
    }

    Scene scene = std::move(sceneResult->first);
    PinholeCamera camera = sceneResult->second.has_value()
                               ? sceneResult->second.value()
                               : PinholeCamera(Point(0, 0, -2.5), 35, 256, 256);

    // Fixed parameters
    const int nPhotons = 50000;             // Fixed photon count
    const double searchRadius = 0.15;       // Search radius for photons
    const int photonSpp = 8;                // Samples per pixel for final rendering
    const unsigned maxPhotonBounces = 8;    // Maximum bounces for photons

    // k values to test: 1, 10, 50, 100
    const std::vector<unsigned> kValues = {1, 10, 50, 100};

    std::cout << "\nFixed Parameters:" << std::endl;
    std::cout << "  - Photon count:         " << nPhotons << std::endl;
    std::cout << "  - Search radius:        " << std::fixed << std::setprecision(3) << searchRadius << std::endl;
    std::cout << "  - Final render SPP:     " << photonSpp << std::endl;
    std::cout << "  - Max photon bounces:   " << maxPhotonBounces << std::endl;
    std::cout << "  - Image resolution:     " << camera.getWidth() << "x" << camera.getHeight() << std::endl;

    // Generate photon map once (fixed for all k values)
    std::cout << "\n[1/2] Generating photon map (" << nPhotons << " photons)..." << std::endl;
    auto t_gen_start = std::chrono::steady_clock::now();
    scene.generarMapaFotones(nPhotons, maxPhotonBounces);
    auto t_gen_end = std::chrono::steady_clock::now();
    double t_gen_ms = std::chrono::duration<double, std::milli>(t_gen_end - t_gen_start).count();
    std::cout << "  Photon map generated in " << std::fixed << std::setprecision(1) << t_gen_ms << " ms" << std::endl;

    std::cout << "\n[2/2] Rendering with varying k values..." << std::endl;
    std::cout << std::string(80, '=') << std::endl;
    std::cout << std::left << std::setw(15) << "k-Neighbors" 
              << std::setw(20) << "Render Time (ms)"
              << std::setw(20) << "Total Time (ms)" << std::endl;
    std::cout << std::string(80, '-') << std::endl;

    std::vector<std::tuple<unsigned, double, double>> results; // k value, render time, total

    for (unsigned k : kValues) {
        std::cout << std::setw(15) << k;
        std::cout.flush();

        // Render with different k value
        KernelEpanechnikov kernel;
        RenderConfig cfgPM(RenderingAlgorithm::PHOTON_MAPPING, photonSpp, RenderingMode::SEQUENTIAL);
        cfgPM.nPaths = nPhotons;
        cfgPM.kPhotons = k;
        cfgPM.radius = searchRadius;
        cfgPM.maxBounces = maxPhotonBounces;
        cfgPM.kernel = &kernel;

        auto t_render_start = std::chrono::steady_clock::now();
        Image pmImg = camera.render(scene, cfgPM);
        auto t_render_end = std::chrono::steady_clock::now();
        double t_render_ms = std::chrono::duration<double, std::milli>(t_render_end - t_render_start).count();
        double t_total_ms = t_gen_ms + t_render_ms;

        std::cout << std::setw(20) << std::fixed << std::setprecision(1) << t_render_ms;
        std::cout << std::setw(20) << std::fixed << std::setprecision(1) << t_total_ms << std::endl;
        std::cout.flush();

        // Save image
        std::string filename = OUTPUT_DIR + "photon_mapping_k" + std::to_string(k) + ".ppm";
        saveToned(filename, pmImg);

        results.push_back(std::make_tuple(k, t_render_ms, t_total_ms));
    }

    // Summary
    std::cout << "\n" << std::string(80, '=') << std::endl;
    std::cout << "=== SUMMARY ===" << std::endl;
    std::cout << std::string(80, '=') << std::endl;
    std::cout << "Photon Map Generation: " << std::fixed << std::setprecision(1) << t_gen_ms << " ms" << std::endl;
    std::cout << std::string(80, '-') << std::endl;

    double fastestRender = std::get<1>(results[0]);
    double slowestRender = std::get<1>(results[0]);
    unsigned fastestK = std::get<0>(results[0]);
    unsigned slowestK = std::get<0>(results[0]);

    for (const auto& result : results) {
        unsigned k = std::get<0>(result);
        double renderTime = std::get<1>(result);
        double totalTime = std::get<2>(result);

        if (renderTime < fastestRender) {
            fastestRender = renderTime;
            fastestK = k;
        }
        if (renderTime > slowestRender) {
            slowestRender = renderTime;
            slowestK = k;
        }

        std::cout << "k=" << std::setw(4) << k 
                  << " | Render: " << std::setw(8) << std::fixed << std::setprecision(1) << renderTime << " ms"
                  << " | Total: " << std::setw(8) << std::fixed << std::setprecision(1) << totalTime << " ms";
        if (k == fastestK) {
            std::cout << " [FASTEST]";
        } else if (k == slowestK) {
            std::cout << " [SLOWEST]";
        }
        std::cout << std::endl;
    }

    double speedupFastestToSlowest = slowestRender / fastestRender;
    std::cout << "\nSpeedup (slowest/fastest): " << std::fixed << std::setprecision(2) << speedupFastestToSlowest << "x" << std::endl;

    std::cout << "\n" << std::string(80, '=') << std::endl;
    std::cout << "Test completed. Images saved to: " << OUTPUT_DIR << std::endl;

    return 0;
}
