#pragma once

#include <optional>
#include <vector>
#include <memory>
#include "geometry.hpp"
#include "kdtree.h"
#include "render_config.hpp"

// Forward declarations
class Object3D;
struct Intersection;
class Ray;
enum class AccelerationStructure;

/**
 * Abstract interface for acceleration structures
 * Allows switching between different spatial data structures for ray-object intersection
 */
class AccelerationStructureInterface {
public:
    virtual ~AccelerationStructureInterface() = default;
    
    // Build the acceleration structure from a list of objects
    virtual void build(const std::vector<std::shared_ptr<Object3D>>& objects) = 0;
    
    // Find the closest intersection with the given ray
    virtual std::optional<Intersection> intersect(const Ray& ray, float maxDistance = 1000.0f) const = 0;
    
    // Check if a ray intersects any object within maxDistance (for shadow rays)
    virtual bool intersectAny(const Ray& ray, float maxDistance) const = 0;
    
    // Get statistics about the acceleration structure
    virtual size_t getObjectCount() const = 0;
    virtual std::string getStats() const = 0;
};

/**
 * No acceleration - simple linear search through all objects
 */
class NoAcceleration : public AccelerationStructureInterface {
private:
    std::vector<std::shared_ptr<Object3D>> objects_;

public:
    void build(const std::vector<std::shared_ptr<Object3D>>& objects) override;
    std::optional<Intersection> intersect(const Ray& ray, float maxDistance = 1000.0f) const override;
    bool intersectAny(const Ray& ray, float maxDistance) const override;
    size_t getObjectCount() const override { return objects_.size(); }
    std::string getStats() const override;
};

/**
 * KD-tree acceleration structure for spatial partitioning
 * Uses the provided kdtree.h implementation
 */
class KDTreeAcceleration : public AccelerationStructureInterface {
private:
    std::vector<std::shared_ptr<Object3D>> objects_;
    
    // For KD-tree, we need to store object centroids as points
    struct ObjectCentroid {
        Point centroid;
        std::shared_ptr<Object3D> object;
        
        ObjectCentroid(const Point& p, std::shared_ptr<Object3D> obj) 
            : centroid(p), object(obj) {}
    };
    
    // Accessor for KD-tree to get 3D coordinates from ObjectCentroid
    struct ObjectCentroidAccessor {
        float operator()(const ObjectCentroid& obj, std::size_t dimension) const {
            switch(dimension) {
                case 0: return obj.centroid.x;
                case 1: return obj.centroid.y;
                case 2: return obj.centroid.z;
                default: return 0.0f;
            }
        }
    };
    
    // KD-tree storing ObjectCentroid entries
    using KDTree = nn::KDTree<ObjectCentroid, 3, ObjectCentroidAccessor>;
    std::unique_ptr<KDTree> kdtree_;

public:
    void build(const std::vector<std::shared_ptr<Object3D>>& objects) override;
    std::optional<Intersection> intersect(const Ray& ray, float maxDistance = 1000.0f) const override;
    bool intersectAny(const Ray& ray, float maxDistance) const override;
    size_t getObjectCount() const override { return objects_.size(); }
    std::string getStats() const override;
    
private:
    Point getObjectCentroid(const std::shared_ptr<Object3D>& object) const;
};

/**
 * Factory for creating acceleration structures
 */
class AccelerationStructureFactory {
public:
    static std::unique_ptr<AccelerationStructureInterface> create(AccelerationStructure type);
};
