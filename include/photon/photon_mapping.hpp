#pragma once

// Main header for the photon mapping module
// This provides a convenient way to include all photon mapping components

#include "photon_map.hpp"
#include "photon_mapper.hpp"
#include "photon_mapping_renderer.hpp"

namespace photon {
    // Re-export main classes for convenience
    using Photon = photon::Photon;
    using PhotonMap = photon::PhotonMap;
    using PhotonMapper = photon::PhotonMapper;
    using PhotonMappingRenderer = photon::PhotonMappingRenderer;
}