#include <iostream>
#include <filesystem>
#include "../include/object3D.hpp"
#include "../include/scene.hpp"
#include "../include/pinholeCamera.hpp"
#include "../include/Image.hpp"
#include "../include/render_config.hpp"
#include "../include/toneMapping.hpp"

static Image applyBilateral(const Image& src, float sigmaSpace, float sigmaColor) {
    Image tmp;
    return tmp.bilateralFilter(src, sigmaSpace, sigmaColor);
}

static void saveGamma(const std::string& path, const Image& img) {
    Image tmp = img;
    ToneMapping::gamma(tmp, 2.2f);
    tmp.writePPM(path);
    std::cout << "  Saved: " << path << std::endl;
}

const std::string OUTPUT_DIR = "test_outputs/emissive_ceiling/";

int main() {
    std::filesystem::create_directories(OUTPUT_DIR);
    
    std::cout << "=== Cornell Box: Point vs Area Light ===" << std::endl;

    // Common materials
    Material white = Material::createPurelyDiffuse(RGB(0.8f, 0.8f, 0.8f));
    Material red = Material::createPurelyDiffuse(RGB(0.8f, 0.1f, 0.1f));
    Material green = Material::createPurelyDiffuse(RGB(0.1f, 0.8f, 0.1f));
    Material glass = Material::createDielectric(1.5f);

    // Camera
    PinholeCamera camera(Point(0, 0, 2.7f), 40, 256, 256, Direction(0, 0, -1));

    // High spp to reduce noise for main renders
    RenderConfig cfg(RenderingAlgorithm::PATH_TRACING, 128, RenderingMode::SEQUENTIAL);
    cfg.maxBounces = 6;

    // ---------- Render 1: Point light ----------
    {
        Scene scenePL(RGB(0.0f, 0.0f, 0.0f));
        // Walls
        scenePL.addObject(std::make_shared<Plane>(Direction(0, -1, 0), white, 1));      // Floor y = -1
        scenePL.addObject(std::make_shared<Plane>(Direction(-1, 0, 0), red, 1));        // Left wall x = -1
        scenePL.addObject(std::make_shared<Plane>(Direction(1, 0, 0), green, 1));       // Right wall x = 1
        scenePL.addObject(std::make_shared<Plane>(Direction(0, 0, -1), white, 1));      // Back wall z = -1
        // Objects
        scenePL.addObject(std::make_shared<Sphere>(Point(-0.35f, -0.7f, -0.2f), 0.3f, white));
        scenePL.addObject(std::make_shared<Sphere>(Point(0.4f, -0.65f, 0.2f), 0.35f, glass));
        // Point light
        scenePL.addLight(std::make_shared<PointLight>(Point(0.0f, 0.8f, 0.0f), RGB(12.0f, 12.0f, 12.0f)));

        std::cout << "\nRendering point light..." << std::endl;
        Image imgPL = camera.render(scenePL, cfg);
        ToneMapping::gamma(imgPL, 2.2f);
        imgPL.writePPM(OUTPUT_DIR + "point_gamma.ppm");
        std::cout << "  Saved: " << OUTPUT_DIR << "point_gamma.ppm" << std::endl;
    }

    // ---------- Render 2: Area (quad) light ----------
    {
        Scene sceneAL(RGB(0.0f, 0.0f, 0.0f));
        // Walls
        sceneAL.addObject(std::make_shared<Plane>(Direction(0, -1, 0), white, 1));      // Floor y = -1
        sceneAL.addObject(std::make_shared<Plane>(Direction(-1, 0, 0), red, 1));        // Left wall x = -1
        sceneAL.addObject(std::make_shared<Plane>(Direction(1, 0, 0), green, 1));       // Right wall x = 1
        sceneAL.addObject(std::make_shared<Plane>(Direction(0, 0, -1), white, 1));      // Back wall z = -1
        // Objects
        sceneAL.addObject(std::make_shared<Sphere>(Point(-0.35f, -0.7f, -0.2f), 0.3f, white));
        sceneAL.addObject(std::make_shared<Sphere>(Point(0.4f, -0.65f, 0.2f), 0.35f, glass));
        // Emissive quad
        Material emissive = Material::createPurelyDiffuse(RGB(0.0f, 0.0f, 0.0f));
        emissive.emission = RGB(18.0f, 18.0f, 18.0f);
        Point ceilingCenter(0.0f, 0.99f, 0.0f);
        Direction u(0.8f, 0.0f, 0.0f);
        Direction v(0.0f, 0.0f, 0.8f); // u×v points down
        sceneAL.addObject(std::make_shared<Quad>(ceilingCenter, u, v, emissive));

        std::cout << "\nRendering area light..." << std::endl;
        Image imgAL = camera.render(sceneAL, cfg);
        saveGamma(OUTPUT_DIR + "area_gamma.ppm", imgAL);

        // Bilateral filtered indirect: render direct+indirect separately
        const int sppDirect = 32;
        const int sppIndirect = 128;
        const float sigmaSpace = 1.5f;
        const float sigmaColor = 0.08f;

        RenderConfig cfgDirect(RenderingAlgorithm::PATH_TRACING, sppDirect, RenderingMode::SEQUENTIAL);
        cfgDirect.maxBounces = 6;
        cfgDirect.lightingDecomposition = LightingDecomposition::DIRECT_ONLY;

        RenderConfig cfgIndirect(RenderingAlgorithm::PATH_TRACING, sppIndirect, RenderingMode::SEQUENTIAL);
        cfgIndirect.maxBounces = 6;
        cfgIndirect.lightingDecomposition = LightingDecomposition::INDIRECT_ONLY;

        std::cout << "Rendering area light DIRECT (" << sppDirect << " spp)..." << std::endl;
        Image direct = camera.render(sceneAL, cfgDirect);
        std::cout << "Rendering area light INDIRECT (" << sppIndirect << " spp)..." << std::endl;
        Image indirect = camera.render(sceneAL, cfgIndirect);

        std::cout << "Applying bilateral filter to INDIRECT (sigmaSpace=" << sigmaSpace << ", sigmaColor=" << sigmaColor << ")..." << std::endl;
        Image indirectFiltered = applyBilateral(indirect, sigmaSpace, sigmaColor);

        Image combinedFiltered(direct.width, direct.height);
        for (size_t i = 0; i < direct.pixels.size(); ++i) {
            combinedFiltered.pixels[i] = direct.pixels[i] + indirectFiltered.pixels[i];
        }

        saveGamma(OUTPUT_DIR + "area_bilateral_gamma.ppm", combinedFiltered);
    }

    std::cout << "\n=== Rendering completed (point + area) ===" << std::endl;
    
    return 0;
}
