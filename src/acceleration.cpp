#include "acceleration.hpp"
#include "object3D.hpp"
#include <iostream>
#include <sstream>
#include <algorithm>

/**
 * NoAcceleration implementation - simple linear search
 */
void NoAcceleration::build(const std::vector<std::shared_ptr<Object3D>>& objects) {
    objects_ = objects;
}

std::optional<Intersection> NoAcceleration::intersect(const Ray& ray, float maxDistance) const {
    std::optional<Intersection> closest_intersection = std::nullopt;
    
    for (const auto& object : objects_) {
        auto intersection = object->intersect(ray);
        if (intersection && 
            intersection->distance < maxDistance &&
            (!closest_intersection || intersection->distance < closest_intersection->distance)) {
            closest_intersection = intersection;
        }
    }
    
    return closest_intersection;
}

bool NoAcceleration::intersectAny(const Ray& ray, float maxDistance) const {
    for (const auto& object : objects_) {
        auto intersection = object->intersect(ray);
        if (intersection && intersection->distance < maxDistance) {
            return true;
        }
    }
    return false;
}

std::string NoAcceleration::getStats() const {
    std::stringstream ss;
    ss << "NoAcceleration: " << objects_.size() << " objects, linear search";
    return ss.str();
}

/**
 * KDTreeAcceleration implementation
 */
void KDTreeAcceleration::build(const std::vector<std::shared_ptr<Object3D>>& objects) {
    objects_ = objects;
    
    if (objects_.empty()) {
        kdtree_.reset();
        return;
    }
    
    // Create ObjectCentroid entries for the KD-tree
    std::vector<ObjectCentroid> centroids;
    centroids.reserve(objects_.size());
    
    for (const auto& object : objects_) {
        Point centroid = getObjectCentroid(object);
        centroids.emplace_back(centroid, object);
    }
    
    // Build the KD-tree
    kdtree_ = std::make_unique<KDTree>(std::move(centroids), ObjectCentroidAccessor());
}

std::optional<Intersection> KDTreeAcceleration::intersect(const Ray& ray, float maxDistance) const {
    if (!kdtree_ || objects_.empty()) {
        return std::nullopt;
    }
    
    // For scenes with few objects, KD-tree overhead isn't worth it
    if (objects_.size() < 50) {
        std::optional<Intersection> closest_intersection = std::nullopt;
        for (const auto& object : objects_) {
            auto intersection = object->intersect(ray);
            if (intersection && 
                intersection->distance < maxDistance &&
                (!closest_intersection || intersection->distance < closest_intersection->distance)) {
                closest_intersection = intersection;
            }
        }
        return closest_intersection;
    }
    
    // For larger scenes, use a more conservative KD-tree query
    std::array<float, 3> queryPoint = {ray.origin.x, ray.origin.y, ray.origin.z};
    
    // Use smaller search radius and fewer objects for efficiency
    size_t numToQuery = std::min(objects_.size(), static_cast<size_t>(10));
    float searchRadius = maxDistance * 0.5f; // More conservative radius
    
    auto nearbyObjects = kdtree_->nearest_neighbors(queryPoint, numToQuery, searchRadius);
    
    std::optional<Intersection> closest_intersection = std::nullopt;
    
    // Test intersections with nearby objects only
    for (const auto* objCentroid : nearbyObjects) {
        auto intersection = objCentroid->object->intersect(ray);
        if (intersection && 
            intersection->distance < maxDistance &&
            (!closest_intersection || intersection->distance < closest_intersection->distance)) {
            closest_intersection = intersection;
        }
    }
    
    // If no nearby objects found, fall back to linear search only if we have many objects
    if (!closest_intersection && objects_.size() > 100) {
        for (const auto& object : objects_) {
            auto intersection = object->intersect(ray);
            if (intersection && 
                intersection->distance < maxDistance &&
                (!closest_intersection || intersection->distance < closest_intersection->distance)) {
                closest_intersection = intersection;
            }
        }
    }
    
    return closest_intersection;
}

bool KDTreeAcceleration::intersectAny(const Ray& ray, float maxDistance) const {
    if (!kdtree_ || objects_.empty()) {
        return false;
    }
    
    // Query nearby objects
    std::array<float, 3> queryPoint = {ray.origin.x, ray.origin.y, ray.origin.z};
    size_t numToQuery = std::min(objects_.size(), static_cast<size_t>(10));
    float searchRadius = maxDistance * 2.0f;
    
    auto nearbyObjects = kdtree_->nearest_neighbors(queryPoint, numToQuery, searchRadius);
    
    // Test nearby objects first
    for (const auto* objCentroid : nearbyObjects) {
        auto intersection = objCentroid->object->intersect(ray);
        if (intersection && intersection->distance < maxDistance) {
            return true;
        }
    }
    
    // Fallback: test all objects if nothing found nearby
    for (const auto& object : objects_) {
        auto intersection = object->intersect(ray);
        if (intersection && intersection->distance < maxDistance) {
            return true;
        }
    }
    
    return false;
}

std::string KDTreeAcceleration::getStats() const {
    std::stringstream ss;
    ss << "KDTreeAcceleration: " << objects_.size() << " objects in spatial tree";
    return ss.str();
}

Point KDTreeAcceleration::getObjectCentroid(const std::shared_ptr<Object3D>& object) const {
    // We need to compute centroids for different object types
    // This is a simplified approach - ideally we'd have virtual methods for this
    
    // For now, we'll try to cast to known types and compute their centroids
    // If we can't determine the centroid, we'll use origin
    
    // Try to cast to Sphere
    if (auto sphere = std::dynamic_pointer_cast<Sphere>(object)) {
        return sphere->center;
    }
    
    // Try to cast to Triangle  
    if (auto triangle = std::dynamic_pointer_cast<Triangle>(object)) {
        return triangle->centroid();
    }
    
    // For other objects (Plane, etc.), use origin as fallback
    // In a more complete implementation, we'd add getBoundingBox() or getCentroid() 
    // virtual methods to Object3D
    return Point(0, 0, 0);
}

/**
 * Factory implementation
 */
std::unique_ptr<AccelerationStructureInterface> AccelerationStructureFactory::create(AccelerationStructure type) {
    switch (type) {
        case AccelerationStructure::NONE:
            return std::make_unique<NoAcceleration>();
        case AccelerationStructure::KDTREE:
            return std::make_unique<KDTreeAcceleration>();
        case AccelerationStructure::BVH:
            // Future implementation
            std::cerr << "BVH not implemented yet, falling back to no acceleration" << std::endl;
            return std::make_unique<NoAcceleration>();
        default:
            return std::make_unique<NoAcceleration>();
    }
}
