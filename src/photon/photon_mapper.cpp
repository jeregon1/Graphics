#include "photon/photon_mapper.hpp"
#include "scene.hpp"
#include "material.hpp"
#include "object3D.hpp"
#include "utils.hpp"
#include "constants.hpp"
#include <iostream>
#include <numeric>
#include <cmath>

namespace photon {

void PhotonMapper::generatePhotonMaps(const Scene& scene, 
                                       int numPhotons, 
                                       unsigned maxBounces) {
    std::list<Photon> regularPhotons;
    std::list<Photon> causticPhotons;
    
    // Get lights from scene
    const auto& lights = scene.getLights();
    
    // Calculate total light emission for energy distribution
    double totalEmission = std::accumulate(lights.begin(), lights.end(), 0.0, 
        [](double sum, const std::shared_ptr<PointLight>& light) {
            return sum + light->getPowerSum();
        });
    
    if (totalEmission <= 0.0) {
        std::cout << "Warning: No light emission found!" << std::endl;
        regularPhotonMap_.clear();
        causticPhotonMap_.clear();
        mapsBuilt_ = false;
        return;
    }
    
    // Generate photons from each light source
    for (const auto& light : lights) {
        generatePhotonsFromLight(*light, numPhotons, totalEmission, maxBounces, 
                                scene, regularPhotons, causticPhotons);
    }
    
    if (regularPhotons.empty() && causticPhotons.empty()) {
        std::cout << "Warning: No photons generated!" << std::endl;
        regularPhotonMap_.clear();
        causticPhotonMap_.clear();
        mapsBuilt_ = false;
        return;
    }
    
    std::cout << "Generated " << regularPhotons.size() << " regular photons and " 
              << causticPhotons.size() << " caustic photons" << std::endl;
    
    // Build the photon maps
    regularPhotonMap_.build(regularPhotons);
    causticPhotonMap_.build(causticPhotons);
    mapsBuilt_ = true;
}

void PhotonMapper::generatePhotonsFromLight(const PointLight& light,
                                           int numPhotons,
                                           double totalEmission,
                                           unsigned maxBounces,
                                           const Scene& scene,
                                           std::list<Photon>& regularPhotons,
                                           std::list<Photon>& causticPhotons) {
    // Distribute photons proportionally to light power
    int numPhotonsFromLight = (int)(numPhotons * light.getPowerSum() / totalEmission);
    
    for (int i = 0; i < numPhotonsFromLight; i++) {
        // Generate random direction
        Direction direction = muestraAleatoriaUniforme();
        Ray ray(light.center, direction);
        
        // Photon flux = 4π * power / numPhotons (conservation of energy)
        RGB photonFlux = light.power * (4 * M_PI) / numPhotonsFromLight;
        
        // Trace the photon through the scene
        tracePhoton(ray, photonFlux, scene, regularPhotons, causticPhotons, 
                   false, maxBounces);
    }
}

void PhotonMapper::tracePhoton(const Ray& initialRay,
                               const RGB& initialFlux,
                               const Scene& scene,
                               std::list<Photon>& regularPhotons,
                               std::list<Photon>& causticPhotons,
                               bool isCaustic,
                               unsigned maxBounces) const {
    Ray currentRay = initialRay;
    RGB currentFlux = initialFlux;
    bool firstBounce = true;
    
    for (unsigned bounce = 0; bounce < maxBounces; bounce++) {
        auto intersection = scene.intersect(currentRay);
        if (!intersection)
            return; // Photon escaped scene
        
        Material material = intersection->material;
        Direction normal = intersection->normal;
        
        // Ensure normal faces incoming ray
        if (currentRay.direction.dot(normal) > 0.0)
            normal = -normal;
        
        // Russian roulette for material interaction
        double totalProb = material.p_diffuse + material.p_specular + material.p_transmittance;
        
        if (totalProb <= 0.0)
            return; // No interaction possible
        
        double probability = rand0_1() * totalProb;
        
        Direction newDirection;
        bool storePhoton = false;
        RGB radiance;
        
        if (probability <= material.p_diffuse) { // Diffuse interaction
            newDirection = randomCosineDirection(normal);
            RGB brdfWeight = material.evaluateBSDF(newDirection, -currentRay.direction, normal);
            storePhoton = true;
            radiance = brdfWeight * utils::cosTheta(normal, newDirection);
            // Don't change caustic flag - if it was caustic, it stays caustic
        }
        else if (probability <= material.p_diffuse + material.p_specular) {
            // Specular reflection - continue tracing, don't store photon
            newDirection = currentRay.direction.specular(normal);
            radiance = material.getSpecular();
            isCaustic = true; // Mark as caustic path
        }
        else if (probability <= material.p_diffuse + material.p_specular + material.p_transmittance) {
            // Transmission/refraction - continue tracing, don't store photon
            auto refracted = material.refract(currentRay.direction, normal);
            if (refracted) {
                newDirection = *refracted;
                radiance = material.getTransmittance();
                isCaustic = true; // Mark as caustic path
            } else
                return; // Total internal reflection, absorb photon
        }
        else {
            return; // Absorption - photon is absorbed
        }
        
        // Store photon only on diffuse surfaces and NOT on first bounce
        if (storePhoton && !firstBounce) {
            // Store with incoming light direction
            Photon photon(intersection->point, currentRay.direction, currentFlux);
            if (isCaustic)
                causticPhotons.push_back(photon);
            else
                regularPhotons.push_back(photon);
        }
        
        // Update photon flux
        currentFlux *= radiance;
        
        // Create new ray for next bounce (offset to avoid self-intersection)
        Point newOrigin = intersection->point + newDirection * EPS;
        currentRay = Ray(newOrigin, newDirection);
        
        firstBounce = false;
        
        // Russian roulette termination based on flux intensity
        if (bounce > 5) {
            double maxFlux = currentFlux.max();
            if (maxFlux < 0.01 || rand0_1() > maxFlux)
                break;
        }
    }
}

void PhotonMapper::clear() {
    regularPhotonMap_.clear();
    causticPhotonMap_.clear();
    mapsBuilt_ = false;
}

} // namespace photon