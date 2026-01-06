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

const std::string OUTPUT_DIR = "test_outputs/photon_scaling/";

void saveToned(const std::string& path, const Image& img) {
    Image tmp = img;
    ToneMapping::gamma(tmp, 2.2f);
    tmp.writePPM(path);
    std::cout << "    Saved: " << path << std::endl;
}

int main() {
    std::filesystem::create_directories(OUTPUT_DIR);

    std::cout << "\n" << std::string(80, '=') << std::endl;
    std::cout << "=== PHOTON MAPPING SCALING TEST ===" << std::endl;
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
    const unsigned kPhotons = 10;           // Fixed k-nearest photons
    const double searchRadius = 0.15;       // Search radius for photons
    const int photonSpp = 8;                // Samples per pixel for final rendering
    const unsigned maxPhotonBounces = 8;    // Maximum bounces for photons

    // Photon counts to test: 1K, 10K, 100K
    const std::vector<int> photonCounts = {1000, 10000, 100000};

    std::cout << "\nFixed Parameters:" << std::endl;
    std::cout << "  - k-nearest photons:    " << kPhotons << std::endl;
    std::cout << "  - Search radius:        " << std::fixed << std::setprecision(3) << searchRadius << std::endl;
    std::cout << "  - Final render SPP:     " << photonSpp << std::endl;
    std::cout << "  - Max photon bounces:   " << maxPhotonBounces << std::endl;
    std::cout << "  - Image resolution:     " << camera.getWidth() << "x" << camera.getHeight() << std::endl;

    std::cout << "\n" << std::string(80, '=') << std::endl;
    std::cout << std::left << std::setw(15) << "Photon Count" 
              << std::setw(20) << "Gen Time (ms)" 
              << std::setw(20) << "Render Time (ms)"
              << std::setw(20) << "Total Time (ms)" << std::endl;
    std::cout << std::string(80, '-') << std::endl;

    std::vector<std::tuple<int, double, double, double>> results; // photon count, gen time, render time, total

    for (int nPhotons : photonCounts) {
        std::cout << std::setw(15) << nPhotons;
        std::cout.flush();

        // Step 1: Generate photon map
        auto t_gen_start = std::chrono::steady_clock::now();
        scene.generarMapaFotones(nPhotons, maxPhotonBounces);
        auto t_gen_end = std::chrono::steady_clock::now();
        double t_gen_ms = std::chrono::duration<double, std::milli>(t_gen_end - t_gen_start).count();

        std::cout << std::setw(20) << std::fixed << std::setprecision(1) << t_gen_ms;
        std::cout.flush();

        // Step 2: Render with photon mapping
        KernelEpanechnikov kernel;
        RenderConfig cfgPM(RenderingAlgorithm::PHOTON_MAPPING, photonSpp, RenderingMode::SEQUENTIAL);
        cfgPM.nPaths = nPhotons;
        cfgPM.kPhotons = kPhotons;
        cfgPM.radius = searchRadius;
        cfgPM.maxBounces = maxPhotonBounces;
        cfgPM.kernel = &kernel;

        auto t_render_start = std::chrono::steady_clock::now();
        Image pmImg = camera.render(scene, cfgPM);
        auto t_render_end = std::chrono::steady_clock::now();
        double t_render_ms = std::chrono::duration<double, std::milli>(t_render_end - t_render_start).count();

        std::cout << std::setw(20) << std::fixed << std::setprecision(1) << t_render_ms;
        std::cout << std::setw(20) << std::fixed << std::setprecision(1) << (t_gen_ms + t_render_ms) << std::endl;
        std::cout.flush();

        // Save image
        std::string filename = OUTPUT_DIR + "photon_mapping_" + std::to_string(nPhotons / 1000) + "k.ppm";
        saveToned(filename, pmImg);

        results.push_back(std::make_tuple(nPhotons, t_gen_ms, t_render_ms, t_gen_ms + t_render_ms));
    }

    // Summary
    std::cout << "\n" << std::string(80, '=') << std::endl;
    std::cout << "=== SUMMARY ===" << std::endl;
    std::cout << std::string(80, '=') << std::endl;

    double fastestTotal = std::get<3>(results[0]);
    int fastestIdx = 0;
    for (size_t i = 1; i < results.size(); ++i) {
        if (std::get<3>(results[i]) < fastestTotal) {
            fastestTotal = std::get<3>(results[i]);
            fastestIdx = i;
        }
    }

    for (size_t i = 0; i < results.size(); ++i) {
        int nPhotons = std::get<0>(results[i]);
        double genTime = std::get<1>(results[i]);
        double renderTime = std::get<2>(results[i]);
        double totalTime = std::get<3>(results[i]);
        double speedup = totalTime / fastestTotal;

        std::cout << "Photons: " << std::setw(6) << nPhotons 
                  << " | Gen: " << std::setw(8) << std::fixed << std::setprecision(1) << genTime << " ms"
                  << " | Render: " << std::setw(8) << std::fixed << std::setprecision(1) << renderTime << " ms"
                  << " | Total: " << std::setw(8) << std::fixed << std::setprecision(1) << totalTime << " ms"
                  << " | Speedup: " << std::fixed << std::setprecision(2) << speedup << "x";
        if (i == fastestIdx) {
            std::cout << " [FASTEST]";
        }
        std::cout << std::endl;
    }

    std::cout << "\n" << std::string(80, '=') << std::endl;
    std::cout << "Test completed. Images saved to: " << OUTPUT_DIR << std::endl;

    return 0;
}
