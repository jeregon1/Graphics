#pragma once

#include "constants.hpp"
#include "RGB.hpp"
#include "geometry.hpp"

#include <optional>
#include <iomanip>

class Direction;

struct Material {
private:
    RGB transmittance; // kt - transmittance coefficient (for refraction)
    RGB diffuse; // kd - diffuse reflectance coefficient
    RGB specular; // ks - specular reflectance coefficient  
    
    static constexpr float p_limit = 0.9f;
    
public:
    RGB emission; // Le - emission (light emitted by material)
    double p_diffuse = 0.0; // Probabilidad de difuso
    double p_specular = 0.0; // Probabilidad de especular
    double p_transmittance = 0.0; // Probabilidad de refracción
    double n = 1; // Índice de refracción

    // Constructor with transparency support
    Material(const RGB& diffuse = RGB{0,0,0}, const RGB& specular = RGB{0,0,0}, const RGB& transmittance = RGB{0,0,0}, const RGB& emission = RGB{0,0,0}) :
            transmittance(transmittance), diffuse(diffuse), specular(specular), emission(emission), n(transmittance.max() > 0 ? 1.5 : 1)
    { 
        // Check if material needs normalization BEFORE normalizing
        bool needsNormalization = !isPhysicallyValid();
        
        // Normalize material coefficients to ensure physical validity
        normalize();
        
        // Emit warning if material needed normalization
        if (needsNormalization) {
            std::cerr << "Warning: Material properties out of range. Auto-normalized:" << std::endl
                      << "  diffuse: " << diffuse << std::endl
                      << "  specular: " << specular << std::endl
                      << "  transmittance: " << transmittance << std::endl;
        }
    }

    // Normalize material coefficients to ensure physical validity
    void normalize();

    std::optional<Direction> refract(const Direction& wo, const Direction& normal) const;

    // Validate material: ensure kd + ks + kt < 1 for all RGB channels and
    bool isPhysicallyValid() const {
        bool achromaticSpecular = (specular.r == specular.g && specular.g == specular.b);
        bool achromaticTransmittance = (transmittance.r == transmittance.g && transmittance.g == transmittance.b);
        bool emissionConflict = isEmissive() && (specular.max() > EPS || transmittance.max() > EPS);
        bool sumValid = (diffuse.r + specular.r + transmittance.r < 1.0f) &&
                        (diffuse.g + specular.g + transmittance.g < 1.0f) &&
                        (diffuse.b + specular.b + transmittance.b < 1.0f);
        return achromaticSpecular && achromaticTransmittance && !emissionConflict && sumValid;
    }

    // Evaluate BSDF according to: fr(x,ωi,ωo) = kd(1/π) + ks(δωr(ωi))/(n·ωi) + kt(δωt(ωi))/(n·ωi)
    // Note: Delta functions (specular reflection/transmittance) are handled through importance sampling in path tracing
    RGB evaluateBSDF(const Direction& wi, const Direction& wo, const Direction& normal) const;

    // Stub for test compatibility
    RGB evaluateBSDFWithDeltas(const Direction& wi, const Direction& wo, const Direction& normal, bool& isSpecular, bool& isRefraction) const {
        isSpecular = false;
        isRefraction = false;
        return getDiffuse() / M_PI;
    }

    // Get perfect reflection direction
    Direction getPerfectReflection(const Direction& wo, const Direction& normal) const {
        return wo - normal * (2.0f * wo.dot(normal));
    }

    bool isEmissive() const {
        return emission.max() > EPS;
    }

    // Check if material is purely diffuse (plastic example)
    bool isPurelyDiffuse() const {
        return specular.max() < EPS && transmittance.max() < EPS;
    }

    // Check if material is plastic (diffuse + specular, no transmittance)
    bool isPlastic() const {
        return transmittance.max() < EPS && diffuse.max() > EPS && specular.max() > EPS;
    }

    // Check if material is dielectric (specular + transmittance, no diffuse)
    bool isDielectric() const {
        return diffuse.max() < EPS && (specular.max() > EPS || transmittance.max() > EPS);
    }

    // Factory methods for common material types
    static Material createPurelyDiffuse(const RGB& color) {
        return Material(color, RGB(0, 0, 0), RGB(0, 0, 0));
    }

    static Material createPlastic(const RGB& diffuseColor, const RGB& specularColor = RGB(0.1f, 0.1f, 0.1f)) {
        return Material(diffuseColor, specularColor, RGB(0, 0, 0));
    }

    static Material createDielectric(float refractionIndex, const RGB& transmissionColor = RGB(0.89f, 0.89f, 0.89f)) {
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
            << ", specular: " << specular.r 
            << ", transmittance: " << transmittance.r 
            << ", emission: " << emission
            << ", p_diffuse: " << p_diffuse 
            << ", p_specular: " << p_specular 
            << ", p_transmittance: " << p_transmittance 
            << ", n: " << n
            << ")";
        return oss.str();
    }

    friend std::ostream& operator<<(std::ostream& os, const Material& material) {
        os << material.toString();
        return os;
    }

    // Setters for material properties
    void setDiffuse(const RGB& color) { diffuse = color; normalize(); }
    RGB getDiffuse() const { return diffuse; }
    void setSpecular(const RGB& color) { specular = color; normalize(); }
    RGB getSpecular() const { return specular; }
    void setTransmittance(const RGB& color) { transmittance = color; normalize();
        if (n != 1.0) return; // Only set n if it hasn't been set yet
        if (transmittance.max() > 0) n = 1.5;
        else n = 1; // Default glass index
    }
    RGB getTransmittance() const { return transmittance; }
};
