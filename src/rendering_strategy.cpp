#include "../include/rendering_strategy.hpp"
#include "../include/pinholeCamera.hpp"
#include "../include/object3D.hpp"
#include <memory>

// Helper for anti-aliased pixel color sampling
namespace {
    template<typename PerRayColorFunc>
    RGB samplePixelColor(const PinholeCamera& camera, const Scene& scene,
                        float x, float y, unsigned samples, const RenderConfig& config,
                        PerRayColorFunc perRayColor) {
        RGB accumulatedColor(0, 0, 0);
        for (unsigned i = 0; i < samples; i++) {
            float x_offset = x + rand0_1();
            float y_offset = y + rand0_1();
            Ray ray = camera.generateRay(x_offset, y_offset);
            accumulatedColor += perRayColor(ray, scene, config);
        }
        return accumulatedColor / samples;
    }
}

RGB RayTracingStrategy::calculatePixelColor(const PinholeCamera& camera, const Scene& scene,
                                           float x, float y, unsigned samples,
                                           const RenderConfig& config) const {
    return samplePixelColor(camera, scene, x, y, samples, config,
        [&camera](const Ray& ray, const Scene& scene, const RenderConfig&) {
            return camera.traceRay(ray, scene);
        }
    );
}

RGB PathTracingStrategy::calculatePixelColor(const PinholeCamera& camera, const Scene& scene,
                                            float x, float y, unsigned samples,
                                            const RenderConfig& config) const {
    return samplePixelColor(camera, scene, x, y, samples, config,
        [&camera](const Ray& ray, const Scene& scene, const RenderConfig&) {
            return camera.tracePath(ray, scene);
        }
    );
}

RGB PhotonMappingStrategy::calculatePixelColor(const PinholeCamera& camera, const Scene& scene,
                                              float x, float y, unsigned samples,
                                              const RenderConfig& config) const {
    return samplePixelColor(camera, scene, x, y, samples, config,
        [](const Ray& ray, const Scene& scene, const RenderConfig& config) {
            auto intersection = scene.intersect(ray);
            if (intersection) {
                if (config.photonMap && config.kernel) {
                    return scene.ecuacionRenderFotones(
                        intersection->point, ray.direction, intersection->material,
                        intersection->normal, *config.photonMap, config.kPhotons,
                        config.radius, false, config.kernel);
                } else {
                    return intersection->material.diffuse;
                }
            }

            return RGB(0, 0, 0);
        }
    );
}

std::unique_ptr<RenderingStrategy> StrategyFactory::createStrategy(RenderingAlgorithm algorithm) {
    switch (algorithm) {
        case RenderingAlgorithm::RAY_TRACING:
            return std::make_unique<RayTracingStrategy>();
        case RenderingAlgorithm::PATH_TRACING:
            return std::make_unique<PathTracingStrategy>();
        case RenderingAlgorithm::PHOTON_MAPPING:
            return std::make_unique<PhotonMappingStrategy>();
        default:
            return std::make_unique<PathTracingStrategy>();
    }
}
