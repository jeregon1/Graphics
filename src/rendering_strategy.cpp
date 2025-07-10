#include "rendering_strategy.hpp"
#include "pinholeCamera.hpp"
#include "object3D.hpp"
#include "scene.hpp"
#include "utils.hpp"
#include "kernel.hpp"
#include "photon/photon_mapping.hpp"
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
                // Get or create photon mapper for the scene
                static std::once_flag photonMapFlag;
                static std::shared_ptr<photon::PhotonMapper> photonMapper;
                static std::shared_ptr<photon::PhotonMappingRenderer> renderer;
                static KernelEpanechnikov defaultKernel;
                
                std::call_once(photonMapFlag, [&scene, &config]() {
                    std::cout << "Generating photon maps using new modular system..." << std::endl;
                    
                    // Create photon mapper and generate photon maps
                    photonMapper = std::make_shared<photon::PhotonMapper>();
                    photonMapper->generatePhotonMaps(scene, config.nPaths, config.maxBounces);
                    
                    // Create photon mapping renderer
                    renderer = std::make_shared<photon::PhotonMappingRenderer>();
                    renderer->setPhotonMapper(photonMapper);
                    
                    std::cout << "Photon maps generated using new modular system!" << std::endl;
                });
                
                if (!photonMapper || !renderer) {
                    // Fallback to direct lighting if photon mapping fails
                    return scene.nextEventEstimation(*intersection);
                }
                
                Kernel* kernel = config.kernel ? config.kernel : &defaultKernel;
                return renderer->renderPixel(ray.direction, *intersection, scene, config, *kernel, config.maxBounces);
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
