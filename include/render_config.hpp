#pragma once

#include "foton.hpp"

// Forward declarations
class Kernel;
enum class RegionType;
enum class QueueType;

enum class RenderingAlgorithm {
    RAY_TRACING,
    PATH_TRACING,
    PHOTON_MAPPING
};

enum class RenderingMode {
    SEQUENTIAL,
    PARALLEL
};

struct RenderConfig {
    RenderingAlgorithm algorithm = RenderingAlgorithm::PATH_TRACING;
    RenderingMode mode = RenderingMode::PARALLEL;  // Default to parallel
    
    // Parallel config fields (instead of including the whole struct)
    RegionType regionType;
    int regionSize = 8;
    int numThreads = 4;
    QueueType queueType;
    
    // Photon mapping specific parameters
    MapaFotones* photonMap = nullptr;
    unsigned kPhotons = 50;
    double radius = 0.1;
    Kernel* kernel = nullptr;
    
    // Constructors for convenience
    RenderConfig();
    
    RenderConfig(RenderingAlgorithm alg) : algorithm(alg) { initDefaults(); }
    
    RenderConfig(RenderingAlgorithm alg, RenderingMode mode) 
        : algorithm(alg), mode(mode) { initDefaults(); }
    
    // Photon mapping constructor
    RenderConfig(MapaFotones* pMap, unsigned k, double r, Kernel* kern)
        : algorithm(RenderingAlgorithm::PHOTON_MAPPING), photonMap(pMap), 
          kPhotons(k), radius(r), kernel(kern) { initDefaults(); }

private:
    void initDefaults();
};
