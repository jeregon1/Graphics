#pragma once

#include "foton.hpp"
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

// Simple array-based toString functions
inline const char* toString(RenderingAlgorithm alg) {
    static const char* names[] = {"RAY_TRACING", "PATH_TRACING", "PHOTON_MAPPING"};
    return names[static_cast<int>(alg)];
}

inline const char* toString(RenderingMode mode) {
    static const char* names[] = {"SEQUENTIAL", "PARALLEL"};
    return names[static_cast<int>(mode)];
}

inline const char* toString(RegionType type) {
    static const char* names[] = {"PIXEL", "LINE", "COLUMN", "RECTANGLE"};
    return names[static_cast<int>(type)];
}

inline const char* toString(QueueType type) {
    static const char* names[] = {"STD_QUEUE", "LOCK_FREE_QUEUE", "WORK_STEALING"};
    return names[static_cast<int>(type)];
}

inline const char* toString(AccelerationStructure acc) {
    static const char* names[] = {"NONE", "KDTREE", "BVH"};
    return names[static_cast<int>(acc)];
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
    Kernel* kernel = nullptr;

    // Constructors for convenience
    RenderConfig() = default;
    
    RenderConfig(RenderingAlgorithm alg, RenderingMode mode = RenderingMode::PARALLEL) 
        : algorithm(alg), mode(mode) {}
    
    // Photon-mapping ctor, delegates algo setting then initializes its own fields
    RenderConfig(MapaFotones* pMap, unsigned k, double r, Kernel* kern)
        : algorithm(RenderingAlgorithm::PHOTON_MAPPING), photonMap(pMap), 
          kPhotons(k), radius(r), kernel(kern) {}
    
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
               ")";
    }
    
    friend std::ostream& operator<<(std::ostream& os, const RenderConfig& config) {
        return os << config.toString();
    }
};
