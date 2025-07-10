# Photon Mapping Modularization

This document describes the modularization of the photon mapping system in the graphics renderer.

## Problem Statement

The original photon mapping implementation had several issues:
1. **Tight coupling**: Photon mapping code was tightly integrated into the Scene class
2. **Mixed responsibilities**: Scene class handled both scene management AND photon mapping
3. **No clear separation**: Photon generation, storage, and querying were all mixed together
4. **Hard to test**: Photon mapping functionality was embedded in Scene, making it hard to test independently
5. **No abstraction**: No clear interface for photon mapping operations

## New Modular Architecture

The new system separates photon mapping into three distinct modules:

### 1. PhotonMap (`include/photon/photon_map.hpp`)
- **Responsibility**: Photon storage and spatial queries
- **Key Features**:
  - Stores photons using a KD-tree for efficient spatial queries
  - Provides methods to build photon maps from photon lists
  - Supports nearest neighbor queries with radius constraints
  - Thread-safe and memory efficient

### 2. PhotonMapper (`include/photon/photon_mapper.hpp`)
- **Responsibility**: Photon generation and ray tracing
- **Key Features**:
  - Generates photons from light sources in a scene
  - Traces photons through the scene using Monte Carlo methods
  - Separates regular photons from caustic photons
  - Handles photon flux calculation and Russian roulette termination

### 3. PhotonMappingRenderer (`include/photon/photon_mapping_renderer.hpp`)
- **Responsibility**: Radiance estimation using photon maps
- **Key Features**:
  - Performs photon density estimation for diffuse surfaces
  - Handles specular and transmission bounces
  - Integrates with existing kernel system for smoothing
  - Supports both regular and caustic photon contributions

## Directory Structure

```
include/photon/
├── photon_map.hpp              # PhotonMap class
├── photon_mapper.hpp           # PhotonMapper class  
├── photon_mapping_renderer.hpp # PhotonMappingRenderer class
└── photon_mapping.hpp          # Main include file

src/photon/
├── photon_map.cpp              # PhotonMap implementation
├── photon_mapper.cpp           # PhotonMapper implementation
└── photon_mapping_renderer.cpp # PhotonMappingRenderer implementation
```

## Key Classes

### Photon Class
```cpp
class Photon {
public:
    Point position;      // Where the photon is stored
    Direction incidentDir; // Direction the photon came from
    RGB flux;           // Photon energy/flux
    
    Photon(Point p, Direction d, RGB f);
    double getPosition(std::size_t i) const; // For KD-tree indexing
};
```

### PhotonMap Class
```cpp
class PhotonMap {
public:
    void build(const std::list<Photon>& photons);
    std::vector<Photon> findNearestPhotons(const Point& queryPoint, 
                                           unsigned long maxPhotons,
                                           float maxRadius) const;
    bool isBuilt() const;
    size_t size() const;
    void clear();
};
```

### PhotonMapper Class
```cpp
class PhotonMapper {
public:
    void generatePhotonMaps(const Scene& scene, int numPhotons, unsigned maxBounces);
    const PhotonMap& getRegularPhotonMap() const;
    const PhotonMap& getCausticPhotonMap() const;
    bool hasPhotonMaps() const;
    void clear();
};
```

## Usage Example

```cpp
// Create photon mapper
auto photonMapper = std::make_shared<photon::PhotonMapper>();

// Generate photon maps
photonMapper->generatePhotonMaps(scene, 10000, 5);

// Create renderer
auto renderer = std::make_shared<photon::PhotonMappingRenderer>();
renderer->setPhotonMapper(photonMapper);

// Use in rendering
RGB color = renderer->renderPixel(viewDirection, intersection, scene, config, kernel, maxBounces);
```

## Benefits

1. **Separation of Concerns**: Each class has a single, well-defined responsibility
2. **Testability**: Each component can be tested independently
3. **Reusability**: PhotonMapper can be used with different rendering strategies
4. **Maintainability**: Easier to understand and modify photon mapping code
5. **Extensibility**: Easy to add new photon mapping algorithms or optimizations
6. **Performance**: Specialized classes can be optimized for their specific tasks

## Backward Compatibility

The original Scene class methods (`generarMapaFotones`, `ecuacionRenderFotones`) are still available but marked as legacy. The new PhotonMappingStrategy automatically uses the new modular system.

## Testing

New tests have been added:
- `test/test_photon_mapping.cpp` - Tests the photon mapping module functionality
- `test/test_photon_render.cpp` - Tests actual rendering with photon mapping

Run tests with:
```bash
make build/test_photon_mapping && ./build/test_photon_mapping
make build/test_photon_render && ./build/test_photon_render
```

## Migration Guide

For users who want to use the new photon mapping system directly:

1. Include the photon mapping header:
```cpp
#include "photon/photon_mapping.hpp"
```

2. Create and configure photon mapper:
```cpp
auto photonMapper = std::make_shared<photon::PhotonMapper>();
photonMapper->generatePhotonMaps(scene, numPhotons, maxBounces);
```

3. Create renderer:
```cpp
auto renderer = std::make_shared<photon::PhotonMappingRenderer>();
renderer->setPhotonMapper(photonMapper);
```

4. Use in rendering loop:
```cpp
RGB color = renderer->renderPixel(viewDir, intersection, scene, config, kernel, bounces);
```

The PhotonMappingStrategy automatically uses this new system, so existing code using `RenderingAlgorithm::PHOTON_MAPPING` will automatically benefit from the improvements.