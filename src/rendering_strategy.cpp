#include "rendering_strategy.hpp"
#include "pinholeCamera.hpp"
#include "object3D.hpp"
#include "scene.hpp"
#include "utils.hpp"
#include "kernel.hpp"
#include <memory>
#include <iostream>
#include <mutex>

// Helper for anti-aliased pixel color sampling
namespace {
    template<typename PerRayColorFunc>
    RGB samplePixelColor(const PinholeCamera& camera, const Scene& scene,
                        float x, float y, const RenderConfig& config,
                        PerRayColorFunc perRayColor) {
        RGB accumulatedColor(0, 0, 0);
        
        for (int i = 0; i < config.samplesPerPixel; i++) {
            
            float x_offset = x + (rand0_1() - 0.5f)*camera.getPixelSizeX();
            float y_offset = y + (rand0_1() - 0.5f)*camera.getPixelSizeY();
            Ray ray = camera.generateRay(x_offset, y_offset);
            accumulatedColor += perRayColor(ray, scene, config);
        }
        return accumulatedColor / config.samplesPerPixel;
    }
}

RGB RayTracingStrategy::calculatePixelColor(const PinholeCamera& camera, const Scene& scene,
                                           float x, float y, const RenderConfig& config) const {
    return samplePixelColor(camera, scene, x, y, config,
        [&camera](const Ray& ray, const Scene& scene, const RenderConfig&) {
            return camera.traceRay(ray, scene);
        }
    );
}

RGB PathTracingStrategy::calculatePixelColor(const PinholeCamera& camera, const Scene& scene,
                                            float x, float y, const RenderConfig& config) const {
    return samplePixelColor(camera, scene, x, y, config,
        [&camera, maxBounces = config.maxBounces](const Ray& ray, const Scene& scene, const RenderConfig&) {
            return camera.tracePath(ray, scene, maxBounces);
        }
    );
}

RGB PhotonMappingStrategy::calculatePixelColor(const PinholeCamera& camera, const Scene& scene,
                                              float x, float y, const RenderConfig& config) const {
    return samplePixelColor(camera, scene, x, y, config,
        [](const Ray& ray, const Scene& scene, const RenderConfig& config) {

            if (auto intersection = scene.intersect(ray)) {
                // Generate photon map on-the-fly if not already built
                static std::once_flag photonMapFlag;
                static KernelEpanechnikov defaultKernel; // Better kernel choice
                
                std::call_once(photonMapFlag, [&scene, &config]() {
                    std::cout << "Generating photon maps..." << std::endl;
                    const_cast<Scene&>(scene).generarMapaFotones(
                        config.nPaths,
                        config.maxBounces
                    );
                    std::cout << "Photon maps generated!" << std::endl;
                });
                
                Kernel* kernel = config.kernel ? config.kernel : &defaultKernel;

                return scene.ecuacionRenderFotones(ray.direction, *intersection, config, *kernel, config.maxBounces);
            }

            return scene.backgroundColor;
        }
    );
}

std::unique_ptr<RenderingStrategy> StrategyFactory::createStrategy(RenderingAlgorithm algorithm) {
    switch (algorithm) {
        case RenderingAlgorithm::RAY_TRACING:
            return std::make_unique<RayTracingStrategy>();
        case RenderingAlgorithm::PHOTON_MAPPING:
            return std::make_unique<PhotonMappingStrategy>();
        case RenderingAlgorithm::PATH_TRACING:
        default:
            return std::make_unique<PathTracingStrategy>();
    }
}
