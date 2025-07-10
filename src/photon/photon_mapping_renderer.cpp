#include "photon/photon_mapping_renderer.hpp"
#include "scene.hpp"
#include "material.hpp"
#include "kernel.hpp"
#include "render_config.hpp"
#include "utils.hpp"
#include "constants.hpp"
#include <cmath>

namespace photon {

RGB PhotonMappingRenderer::renderPixel(const Direction& viewDirection,
                                       const Intersection& intersection,
                                       const Scene& scene,
                                       const RenderConfig& config,
                                       const Kernel& kernel,
                                       int maxBounces) const {
    if (!photonMapper_ || !photonMapper_->hasPhotonMaps()) {
        // Fallback to direct lighting only if no photon maps
        return estimateDirectLighting(intersection, scene);
    }
    
    return estimateRadiance(viewDirection, intersection, scene, config, kernel, maxBounces);
}

RGB PhotonMappingRenderer::estimateRadiance(const Direction& viewDirection,
                                            const Intersection& intersection,
                                            const Scene& scene,
                                            const RenderConfig& config,
                                            const Kernel& kernel,
                                            int maxBounces) const {
    if (maxBounces < 0)
        return RGB(0, 0, 0); // No more bounces left
    
    Material material = intersection.material;
    
    // Base case: emissive materials
    if (material.isEmissive())
        return material.emission;
    
    // Russian roulette for BSDF sampling
    float pd = material.p_diffuse;
    float ps = material.p_specular;
    float pt = material.p_transmittance;
    float sum = pd + ps + pt;
    if (sum <= 0.0f)
        return RGB(0, 0, 0);
    
    float r = rand0_1() * sum;
    
    // Diffuse branch: photon density estimation + next event
    if (r < pd) {
        RGB Ld(0, 0, 0);
        Direction normal = intersection.normal;
        if (viewDirection.dot(normal) > 0.0f)
            normal = -normal;
        
        // Gather nearby regular photons
        auto regularPhotons = photonMapper_->getRegularPhotonMap().findNearestPhotons(
            intersection.point,
            config.kPhotons,
            config.radius
        );
        
        // Gather nearby caustic photons (with higher importance)
        auto causticPhotons = photonMapper_->getCausticPhotonMap().findNearestPhotons(
            intersection.point,
            config.kPhotons,
            config.radius
        );
        
        RGB regularContrib(0, 0, 0);
        RGB causticContrib(0, 0, 0);
        
        // Process regular photons
        for (const auto& photon : regularPhotons) {
            Direction wi = -photon.incidentDir;
            double cosT = utils::cosTheta(normal, wi);
            if (cosT > 0.0) {
                double d = (photon.position - intersection.point).mod();
                double w = kernel.evaluar(d, config.radius);
                RGB brdf = material.evaluateBSDF(wi, -viewDirection, normal);
                regularContrib += brdf * photon.flux * cosT * w;
            }
        }
        
        // Process caustic photons with higher weight (3x importance)
        const float causticWeight = 3.0f;
        for (const auto& photon : causticPhotons) {
            Direction wi = -photon.incidentDir;
            double cosT = utils::cosTheta(normal, wi);
            if (cosT > 0.0) {
                double d = (photon.position - intersection.point).mod();
                double w = kernel.evaluar(d, config.radius);
                RGB brdf = material.evaluateBSDF(wi, -viewDirection, normal);
                causticContrib += brdf * photon.flux * cosT * w * causticWeight;
            }
        }
        
        // Normalize by search radius area
        double area = M_PI * config.radius * config.radius;
        Ld = (regularContrib + causticContrib) / area;
        
        // Always add direct lighting
        RGB direct = estimateDirectLighting(intersection, scene);
        
        return (Ld + direct) / pd;
    }
    // Specular branch
    else if (r < pd + ps) {
        Direction dir = viewDirection.specular(intersection.normal);
        Point orig = intersection.point + dir * EPS;
        if (auto hit = scene.intersect(Ray(orig, dir))) {
            RGB Li = estimateRadiance(dir, *hit, scene, config, kernel, maxBounces - 1);
            return Li * material.getSpecular() / ps;
        }
    }
    // Transmission branch
    else if (r < pd + ps + pt) {
        auto refr = material.refract(viewDirection, intersection.normal);
        if (!refr)
            return RGB(0, 0, 0);
        Direction dir = *refr;
        Point orig = intersection.point + dir * EPS;
        if (auto hit = scene.intersect(Ray(orig, dir))) {
            RGB Li = estimateRadiance(dir, *hit, scene, config, kernel, maxBounces - 1);
            return Li * material.getTransmittance() / pt;
        }
    }
    return RGB(0, 0, 0);
}

RGB PhotonMappingRenderer::estimateDirectLighting(const Intersection& intersection,
                                                  const Scene& scene) const {
    // Use the scene's next event estimation for direct lighting
    return scene.nextEventEstimation(intersection);
}

} // namespace photon