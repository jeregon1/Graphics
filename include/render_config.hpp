#pragma once

#include "foton.hpp"
#include "toneMapping.hpp"
#include <optional>

// Forward declarations
class Kernel;

// Enhanced enums with built-in string conversion
enum class RenderingAlgorithm {
    RAY_TRACING,
    PATH_TRACING,
    PHOTON_MAPPING
};

enum class RenderingMode {
    SEQUENTIAL,
    PARALLEL
};

enum class RegionType {
    PIXEL,      // Individual pixels
    LINE,       // Horizontal lines  
    COLUMN,     // Vertical columns
    RECTANGLE   // Rectangular blocks
};

enum class QueueType {
    STD_QUEUE,          // Standard queue with mutex
    LOCK_FREE_QUEUE,    // Future: lock-free implementation
    WORK_STEALING       // Future: work-stealing queue
};

enum class AccelerationStructure {
    NONE,           // Linear search through all objects
    KDTREE,         // KD-tree spatial partitioning
    BVH             // Future: Bounding Volume Hierarchy
};

enum class ToneMappingType {
    NONE,
    CLAMP,
    EQUALIZATION,
    EQUALIZATION_GAMMA,
    EQUALIZATION_CLAMP,
    GAMMA,
    CLAMP_GAMMA,
    REINHARD
};

enum class LightingDecomposition {
    NONE,           // Standard rendering (combined direct + indirect)
    DIRECT_ONLY,    // Only direct lighting
    INDIRECT_ONLY,  // Only indirect lighting
    SEPARATE        // Separate images for direct and indirect
};

// Simple array-based toString functions
inline const char* toString(RenderingAlgorithm alg) {
    static const char* names[] = {"ray_tracing", "path_tracing", "photon_mapping"};
    return names[static_cast<int>(alg)];
}

inline const char* toString(RenderingMode mode) {
    static const char* names[] = {"sequential", "parallel"};
    return names[static_cast<int>(mode)];
}

inline const char* toString(RegionType type) {
    static const char* names[] = {"pixel", "line", "column", "rectangle"};
    return names[static_cast<int>(type)];
}

inline const char* toString(QueueType type) {
    static const char* names[] = {"std_queue", "lock_free_queue", "work_stealing"};
    return names[static_cast<int>(type)];
}

inline const char* toString(AccelerationStructure acc) {
    static const char* names[] = {"none", "kdtree", "bvh"};
    return names[static_cast<int>(acc)];
}

inline const char* toString(ToneMappingType type) {
    static const char* names[] = {"none", "clamp", "equalization", "equalization_gamma", "equalization_clamp", "gamma", "clamp_gamma", "reinhard"};
    return names[static_cast<int>(type)];
}

inline const char* toString(LightingDecomposition decomp) {
    static const char* names[] = {"none", "direct_only", "indirect_only", "separate"};
    return names[static_cast<int>(decomp)];
}

// Operator<< overloads for enums
inline std::ostream& operator<<(std::ostream& os, RenderingAlgorithm alg) {
    return os << toString(alg);
}

inline std::ostream& operator<<(std::ostream& os, RenderingMode mode) {
    return os << toString(mode);
}

inline std::ostream& operator<<(std::ostream& os, RegionType type) {
    return os << toString(type);
}

inline std::ostream& operator<<(std::ostream& os, QueueType type) {
    return os << toString(type);
}

inline std::ostream& operator<<(std::ostream& os, AccelerationStructure acc) {
    return os << toString(acc);
}

inline std::ostream& operator<<(std::ostream& os, ToneMappingType type) {
    return os << toString(type);
}

inline std::ostream& operator<<(std::ostream& os, LightingDecomposition decomp) {
    return os << toString(decomp);
}


struct RenderConfig {
    RenderingAlgorithm algorithm = RenderingAlgorithm::PATH_TRACING;
    RenderingMode mode = RenderingMode::PARALLEL;  // Default to parallel
    AccelerationStructure acceleration = AccelerationStructure::NONE;  // Default to no acceleration
    
    // Parallel config fields (instead of including the whole struct)
    RegionType regionType = RegionType::COLUMN;
    int regionSize = 8;
    int numThreads = 4;
    QueueType queueType = QueueType::STD_QUEUE;

    // Photon mapping specific parameters
    MapaFotones* photonMap = nullptr;
    unsigned kPhotons = 50;
    double radius = 0.1;
    unsigned nPaths = 10000;  // Number of photons to emit
    Kernel* kernel = nullptr;
    unsigned maxBounces = 6; // Maximum photon bounces

    // Additional rendering parameters
    int samplesPerPixel = 10;
    bool verbose = false; // Progress bar display
    
    // Lighting decomposition configuration
    LightingDecomposition lightingDecomposition = LightingDecomposition::NONE;
    
    // Tone mapping configuration
    ToneMappingType toneMapping = ToneMappingType::NONE;
    float toneMappingMax = DEFAULT_TONE_MAPPING_MAX;      // For clamp operations
    float toneMappingGamma = DEFAULT_TONE_MAPPING_GAMMA;  // For gamma operations
    float toneMappingKey = DEFAULT_TONE_MAPPING_KEY;       // For Reinhard
    float toneMappingLwhite = DEFAULT_TONE_MAPPING_LWHITE;// For Reinhard
    
    // Bilateral filter configuration (only applied to indirect lighting)
    bool useBilateralFilter = false;  // Enable bilateral filtering on indirect lighting
    float bilateralSigmaSpace = 3.0f;  // Spatial range for bilateral filter
    float bilateralSigmaColor = 0.2f;  // Color range for bilateral filter

    // Constructors for convenience
    RenderConfig(RenderingAlgorithm alg = RenderingAlgorithm::RAY_TRACING, unsigned samplesPerPixel = 4, RenderingMode mode = RenderingMode::PARALLEL) 
        : algorithm(alg), mode(mode), samplesPerPixel(samplesPerPixel) {}
    
    // Photon-mapping ctor, delegates algo setting then initializes its own fields
    RenderConfig(MapaFotones* pMap, unsigned k, double r, unsigned nP, Kernel* kern, unsigned mb = 10)
        : algorithm(RenderingAlgorithm::PHOTON_MAPPING), photonMap(pMap), 
          kPhotons(k), radius(r), nPaths(nP), kernel(kern), maxBounces(mb) {}
    
    // YAML file support
    static std::optional<RenderConfig> fromYAML(const std::string& filename);
    void saveToYAML(const std::string& filename) const;

    std::string toString() const {
        return std::string("RenderConfig(algorithm: ") + ::toString(algorithm) +
               ", mode: " + ::toString(mode) +
               ", acceleration: " + ::toString(acceleration) +
               ", regionType: " + ::toString(regionType) +
               ", regionSize: " + std::to_string(regionSize) +
               ", numThreads: " + std::to_string(numThreads) +
               ", queueType: " + ::toString(queueType) +
               ", samplesPerPixel: " + std::to_string(samplesPerPixel) +
               ", toneMapping: " + ::toString(toneMapping) +
               ", bilateralFilter: " + (useBilateralFilter ? "true" : "false") +
               (useBilateralFilter ? (std::string(" (space=") + std::to_string(bilateralSigmaSpace) + 
                                     ", color=" + std::to_string(bilateralSigmaColor) + ")") : "") +
               ", verbose: " + (verbose ? "true" : "false") +
               ")";
    }
    
    friend std::ostream& operator<<(std::ostream& os, const RenderConfig& config) {
        return os << config.toString();
    }
};
