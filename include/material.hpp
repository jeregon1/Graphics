#pragma once

#include "constants.hpp"
#include "RGB.hpp"
#include "geometry.hpp"

#include <optional>
#include <iomanip>

class Direction;

struct Material {
    RGB diffuse; // kd - diffuse reflectance coefficient
    RGB specular; // ks - specular reflectance coefficient  
    RGB transmittance; // kt - transmittance coefficient (for refraction)
    double p_diffuse = 0.0; // Probabilidad de difuso
    double p_specular = 0.0; // Probabilidad de especular
    double p_transmittance = 0.0; // Probabilidad de refracción
    double n = 1.0; // Índice de refracción
    RGB emission; // Le - emission (light emitted by material)

    static constexpr float p_limit = 0.9f;

    // Constructor with transparency support
    Material(const RGB& diffuse = RGB{0,0,0}, const RGB& specular = RGB{0,0,0}, const RGB& transmittance = RGB{0,0,0}, const RGB& emission = RGB{0,0,0}) :
            diffuse(diffuse), specular(specular), transmittance(transmittance), emission(emission) 
    { 
        if (!isPhysicallyValid())
            throw std::invalid_argument("Invalid material properties");
        // Set default refractive index for glass if transmittance is non-zero
        if (transmittance.max() > 0.0f) {
            n = 1.5; // Glass refractive index
        }
        normalize(); 
    }

    // Normalize material coefficients to ensure physical validity
    void normalize();

    std::optional<Direction> refractar(const Direction& wo, const Direction& normal) const;

    // Validate material: ensure kd + ks + kt < 1 for all RGB channels and
    bool isPhysicallyValid() const {
        return (specular.r == specular.g && specular.g == specular.b) && // Ensure specular is achromatic
               (transmittance.r == transmittance.g && transmittance.g == transmittance.b) && // Ensure transmittance is achromatic
               !(isEmissive() && specular.max() > EPSILON);
    }

    // Evaluate BSDF according to: fr(x,ωi,ωo) = kd(1/π) + ks(δωr(ωi))/(n·ωi) + kt(δωt(ωi))/(n·ωi)
    // Note: Delta functions (specular reflection/transmittance) are handled through importance sampling in path tracing
    RGB evaluateBSDF(const Direction& wi, const Direction& wo, const Direction& normal) const;

    // Get perfect reflection direction
    Direction getPerfectReflection(const Direction& wo, const Direction& normal) const {
        return wo - normal * (2.0f * wo.dot(normal));
    }

    bool isEmissive() const {
        return emission.max() > EPSILON;
    }

    // Check if material is purely diffuse (plastic example)
    bool isPurelyDiffuse() const {
        return specular.max() < EPSILON && transmittance.max() < EPSILON;
    }

    // Check if material is plastic (diffuse + specular, no transmittance)
    bool isPlastic() const {
        return transmittance.max() < EPSILON && diffuse.max() > EPSILON && specular.max() > EPSILON;
    }

    // Check if material is dielectric (specular + transmittance, no diffuse)
    bool isDielectric() const {
        return diffuse.max() < EPSILON && (specular.max() > EPSILON || transmittance.max() > EPSILON);
    }

    // Factory methods for common material types
    static Material createPurelyDiffuse(const RGB& color) {
        return Material(color, RGB(0, 0, 0), RGB(0, 0, 0));
    }

    static Material createPlastic(const RGB& diffuseColor, const RGB& specularColor = RGB(0.1f, 0.1f, 0.1f)) {
        return Material(diffuseColor, specularColor, RGB(0, 0, 0));
    }

    static Material createDielectric(float refractionIndex, const RGB& transmissionColor = RGB(0.9f, 0.9f, 0.9f)) {
        Material mat(RGB(0, 0, 0), RGB(0.1f, 0.1f, 0.1f), transmissionColor);
        mat.n = refractionIndex;
        return mat;
    }

    // Equality operator for comparing materials
    bool operator==(const Material& other) const {
        return diffuse == other.diffuse && 
               specular == other.specular && 
               transmittance == other.transmittance && 
               emission == other.emission;
    }

    std::string toString() const {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(3);
        oss << "Material(diffuse: " << diffuse 
            << ", specular: " << specular 
            << ", transmittance: " << transmittance 
            << ", emission: " << emission
            << ", p_diffuse: " << p_diffuse 
            << ", p_specular: " << p_specular 
            << ", p_transmittance: " << p_transmittance 
            << ", n: " << n
            << ")";
        return oss.str();
    }
};
