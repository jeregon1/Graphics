#include "../include/render_config.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <optional>

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
    file << "algorithm: ";
    switch (algorithm) {
        case RenderingAlgorithm::RAY_TRACING: file << "ray_tracing"; break;
        case RenderingAlgorithm::PATH_TRACING: file << "path_tracing"; break;
        case RenderingAlgorithm::PHOTON_MAPPING: file << "photon_mapping"; break;
    }
    file << "\n";
    
    // Mode
    file << "mode: ";
    switch (mode) {
        case RenderingMode::SEQUENTIAL: file << "sequential"; break;
        case RenderingMode::PARALLEL: file << "parallel"; break;
    }
    file << "\n";
    
    // Acceleration
    file << "acceleration: ";
    switch (acceleration) {
        case AccelerationStructure::NONE: file << "none"; break;
        case AccelerationStructure::KDTREE: file << "kdtree"; break;
        case AccelerationStructure::BVH: file << "bvh"; break;
    }
    file << "\n\n";
    
    // Parallel config
    file << "# Parallel rendering settings\n";
    file << "region_type: ";
    switch (regionType) {
        case RegionType::PIXEL: file << "pixel"; break;
        case RegionType::LINE: file << "line"; break;
        case RegionType::COLUMN: file << "column"; break;
        case RegionType::RECTANGLE: file << "rectangle"; break;
    }
    file << "\n";
    
    file << "region_size: " << regionSize << "\n";
    file << "num_threads: " << numThreads << "\n";
    
    file << "queue_type: ";
    switch (queueType) {
        case QueueType::STD_QUEUE: file << "std_queue"; break;
        case QueueType::LOCK_FREE_QUEUE: file << "lock_free_queue"; break;
        case QueueType::WORK_STEALING: file << "work_stealing"; break;
    }
    file << "\n\n";
    
    // Photon mapping
    file << "# Photon mapping settings\n";
    file << "k_photons: " << kPhotons << "\n";
    file << "radius: " << radius << "\n";
    
    std::cout << "Render config saved to: " << filename << std::endl;
}
