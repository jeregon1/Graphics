#include "render_config.hpp"
#include "kernel.hpp"
#include "foton.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <optional>
#include <algorithm>

namespace {
    // Parse enum from string
    RenderingAlgorithm parseAlgorithm(const std::string& str) {
        if (str == "ray_tracing" || str == "RAY_TRACING") return RenderingAlgorithm::RAY_TRACING;
        if (str == "path_tracing" || str == "PATH_TRACING") return RenderingAlgorithm::PATH_TRACING;
        if (str == "photon_mapping" || str == "PHOTON_MAPPING") return RenderingAlgorithm::PHOTON_MAPPING;
        return RenderingAlgorithm::PATH_TRACING; // default
    }
    
    RenderingMode parseMode(const std::string& str) {
        if (str == "sequential" || str == "SEQUENTIAL") return RenderingMode::SEQUENTIAL;
        if (str == "parallel" || str == "PARALLEL") return RenderingMode::PARALLEL;
        return RenderingMode::PARALLEL; // default
    }
    
    AccelerationStructure parseAcceleration(const std::string& str) {
        if (str == "none" || str == "NONE") return AccelerationStructure::NONE;
        if (str == "kdtree" || str == "KDTREE") return AccelerationStructure::KDTREE;
        if (str == "bvh" || str == "BVH") return AccelerationStructure::BVH;
        return AccelerationStructure::NONE; // default
    }
    
    RegionType parseRegionType(const std::string& str) {
        if (str == "pixel" || str == "PIXEL") return RegionType::PIXEL;
        if (str == "line" || str == "LINE") return RegionType::LINE;
        if (str == "column" || str == "COLUMN") return RegionType::COLUMN;
        if (str == "rectangle" || str == "RECTANGLE") return RegionType::RECTANGLE;
        return RegionType::COLUMN; // default
    }
    
    QueueType parseQueueType(const std::string& str) {
        if (str == "std_queue" || str == "STD_QUEUE") return QueueType::STD_QUEUE;
        if (str == "lock_free_queue" || str == "LOCK_FREE_QUEUE") return QueueType::LOCK_FREE_QUEUE;
        if (str == "work_stealing" || str == "WORK_STEALING") return QueueType::WORK_STEALING;
        return QueueType::STD_QUEUE; // default
    }
    
    ToneMappingType parseToneMapping(const std::string& str) {
        if (str == "none" || str == "NONE") return ToneMappingType::NONE;
        if (str == "clamp" || str == "CLAMP") return ToneMappingType::CLAMP;
        if (str == "equalization" || str == "EQUALIZATION") return ToneMappingType::EQUALIZATION;
        if (str == "equalization_gamma" || str == "EQUALIZATION_GAMMA") return ToneMappingType::EQUALIZATION_GAMMA;
        if (str == "equalization_clamp" || str == "EQUALIZATION_CLAMP") return ToneMappingType::EQUALIZATION_CLAMP;
        if (str == "gamma" || str == "GAMMA") return ToneMappingType::GAMMA;
        if (str == "clamp_gamma" || str == "CLAMP_GAMMA") return ToneMappingType::CLAMP_GAMMA;
        if (str == "reinhard" || str == "REINHARD") return ToneMappingType::REINHARD;
        return ToneMappingType::NONE; // default
    }
    
    // Parse kernel name into Kernel* (or nullptr)
    Kernel* parseKernel(const std::string& s) {
        std::string name = s;
        std::transform(name.begin(), name.end(), name.begin(), ::tolower);
        if (name == "box")           return new KernelCaja;
        if (name == "triangular")    return new KernelTriangular;
        if (name == "gaussian")      return new KernelGaussiano;
        if (name == "epanechnikov")  return new KernelEpanechnikov;
        if (name == "quartic")       return new KernelQuartic;
        if (name == "tripeso")       return new KernelTripeso;
        if (name == "tricubo")       return new KernelTricubo;
        if (name == "coseno")        return new KernelCoseno;
        if (name == "logistico")     return new KernelLogistico;
        if (name == "sigmoide")      return new KernelSigmoide;
        return nullptr;
    }
}


std::optional<RenderConfig> RenderConfig::fromYAML(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open render config file: " << filename << std::endl;
        return std::nullopt;
    }
    
    RenderConfig config; // Use default values
    std::string line;
    
    while (std::getline(file, line)) {
        // Skip empty lines and comments
        size_t first = line.find_first_not_of(" \t");
        if (first == std::string::npos || line[first] == '#') continue;
        
        line = line.substr(first);
        std::istringstream lineStream(line);
        std::string key;
        
        if (!(lineStream >> key)) continue;
        
        // Remove trailing colon if present
        if (!key.empty() && key.back() == ':') {
            key.pop_back();
        }
        
        if (key == "algorithm") {
            std::string value;
            if (lineStream >> value) {
                config.algorithm = parseAlgorithm(value);
            }
        } else if (key == "mode") {
            std::string value;
            if (lineStream >> value) {
                config.mode = parseMode(value);
            }
        } else if (key == "samples_per_pixel" || key == "samplesPerPixel") {
            unsigned value;
            if (lineStream >> value) {
                config.samplesPerPixel = value;
            }
        } else if (key == "acceleration") {
            std::string value;
            if (lineStream >> value) {
                config.acceleration = parseAcceleration(value);
            }
        } else if (key == "region_type") {
            std::string value;
            if (lineStream >> value) {
                config.regionType = parseRegionType(value);
            }
        } else if (key == "region_size") {
            int value;
            if (lineStream >> value) {
                config.regionSize = value;
            }
        } else if (key == "tone_mapping" || key == "toneMapping") {
            std::string value;
            float param = 0.0f;  // Optional parameter (gamma, key, max, etc)
            if (lineStream >> value) {
                config.toneMapping = parseToneMapping(value);
                // Try to read optional parameter (gamma, key, max, etc)
                if (lineStream >> param) {
                    // Assign parameter based on tone mapping type
                    if (config.toneMapping == ToneMappingType::GAMMA ||
                        config.toneMapping == ToneMappingType::EQUALIZATION_GAMMA ||
                        config.toneMapping == ToneMappingType::CLAMP_GAMMA) {
                        config.toneMappingGamma = param;
                    } else if (config.toneMapping == ToneMappingType::REINHARD) {
                        config.toneMappingKey = param;
                    } else if (config.toneMapping == ToneMappingType::CLAMP ||
                               config.toneMapping == ToneMappingType::EQUALIZATION_CLAMP) {
                        config.toneMappingMax = param;
                    }
                }
            }
        } else if (key == "tone_mapping_max" || key == "toneMappingMax") {
            float value;
            if (lineStream >> value) {
                config.toneMappingMax = value;
            }
        } else if (key == "tone_mapping_gamma" || key == "toneMappingGamma") {
            float value;
            if (lineStream >> value) {
                config.toneMappingGamma = value;
            }
        } else if (key == "tone_mapping_key" || key == "toneMappingKey") {
            float value;
            if (lineStream >> value) {
                config.toneMappingKey = value;
            }
        } else if (key == "tone_mapping_lwhite" || key == "toneMappingLwhite") {
            float value;
            if (lineStream >> value) {
                config.toneMappingLwhite = value;
            }
        } else if (key == "num_threads") {
            int value;
            if (lineStream >> value) {
                config.numThreads = value;
            }
        } else if (key == "queue_type") {
            std::string value;
            if (lineStream >> value) {
                config.queueType = parseQueueType(value);
            }
        } else if (key == "k_photons") {
            unsigned value;
            if (lineStream >> value) {
                config.kPhotons = value;
            }
        } else if (key == "radius") {
            double value;
            if (lineStream >> value) {
                config.radius = value;
            }
        } else if (key == "n_paths" || key == "nPaths") {
            unsigned value;
            if (lineStream >> value) {
                config.nPaths = value;
            }
        } else if (key == "kernel") {
            std::string value;
            if (lineStream >> value) {
                config.kernel = parseKernel(value);
            }
        } else if (key == "max_bounces") {
            lineStream >> config.maxBounces;
        } else if (key == "verbose") {
            std::string value;
            if (lineStream >> value) {
                config.verbose = (value == "true" || value == "1");
            }
        } else if (key == "bilateral_filter" || key == "bilateralFilter") {
            std::string value;
            if (lineStream >> value) {
                config.useBilateralFilter = (value == "true" || value == "1" || value == "yes");
            }
        } else if (key == "bilateral_sigma_space" || key == "bilateralSigmaSpace") {
            float value;
            if (lineStream >> value) {
                config.bilateralSigmaSpace = value;
            }
        } else if (key == "bilateral_sigma_color" || key == "bilateralSigmaColor") {
            float value;
            if (lineStream >> value) {
                config.bilateralSigmaColor = value;
            }
        }
    }
    
    return config;
}

void RenderConfig::saveToYAML(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file for writing: " << filename << std::endl;
        return;
    }
    
    file << "# Render Configuration\n";
    file << "# Generated automatically\n\n";
    
    // Algorithm
    file << "samples_per_pixel: " << samplesPerPixel << "\n";
    file << "algorithm: " << algorithm << "\n";
    file << "\n";
    
    // Mode
    file << "mode: " << mode << "\n";
    file << "\n";
    
    // Acceleration
    file << "acceleration: " << acceleration << "\n";
    file << "\n\n";
    
    // Parallel config
    file << "# Parallel rendering settings\n";
    file << "region_type: " << regionType << "\n";
    file << "\n";
    
    file << "region_size: " << regionSize << "\n";
    file << "num_threads: " << numThreads << "\n";
    
    file << "queue_type: " << queueType << "\n";
    file << "\n";
    
    // Photon mapping
    file << "# Photon mapping settings\n";
    file << "k_photons: " << kPhotons << "\n";
    file << "radius: " << radius << "\n";
    file << "n_paths: " << nPaths << "\n";
    file << "kernel: ";
    if (kernel) file << *kernel;
    else file << "epanechnikov";
    file << "\n";
    file << "max_bounces: " << maxBounces << "\n";
    file << "\n";
    
    // Tone mapping
    file << "# Tone mapping settings\n";
    file << "tone_mapping: " << toneMapping << "\n";
    file << "tone_mapping_max: " << toneMappingMax << "\n";
    file << "tone_mapping_gamma: " << toneMappingGamma << "\n";
    file << "tone_mapping_key: " << toneMappingKey << "\n";
    file << "tone_mapping_lwhite: " << toneMappingLwhite << "\n";
    file << "\n";
    
    // General settings
    file << "verbose: " << (verbose ? "true" : "false") << "\n\n";
    
    // Bilateral filter settings
    file << "# Bilateral filter settings (applied to indirect lighting only)\n";
    file << "bilateral_filter: " << (useBilateralFilter ? "true" : "false") << "\n";
    file << "bilateral_sigma_space: " << bilateralSigmaSpace << "\n";
    file << "bilateral_sigma_color: " << bilateralSigmaColor << "\n\n";
    
    std::cout << "Render config saved to: " << filename << std::endl;
}
