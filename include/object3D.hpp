#pragma once

#include <optional>
#include <string>
#include <fstream>
#include <cmath>

#include "geometry.hpp"
#include "RGB.hpp"
#include "constants.hpp"
#include "material.hpp"

// Structure to return light sampling information
struct LightSample {
    Point position;      // Sampled point on the light surface
    Direction normal;    // Normal at the sampled point
    RGB emission;        // Light emission at that point
    float pdf;           // Probability density of sampling that point
};

struct Intersection {
    float distance;
    Point point;
    Direction normal;
    Material material;

    Intersection(const float distance, const Point& point, const Direction& normal, const Material& material) :
        distance(distance), point(point), normal(normal.normalize()), material(material) {}
};

class Ray {
public:
    Point origin;
    Direction direction;

    Ray(const Point& origin, const Direction& direction):
        origin(origin), direction(direction.normalize()) {}

    // Returns the point at a distance t from the origin
    Point at(float t) const;
};

class Object3D {
public:

    Material material;

    Object3D(const Material& material) : material(material) {}

    virtual std::optional<Intersection> intersect(const Ray& ray) const = 0;   
    
    // Area light support
    virtual float area() const { return 0.0f; }  // Default: not an area light
    virtual std::optional<LightSample> sampleLightPoint() const { return std::nullopt; }
    
    // Accessor for material
    const Material& getMaterial() const { return material; }

    virtual std::string toString() const = 0;
    friend std::ostream& operator<<(std::ostream& os, const Object3D& obj) {
        return os << obj.toString();
    }
};

class PointLight {
public:

    Point center;
    RGB power;

    PointLight(const Point& center, const RGB& emission) : center(center), power(emission) {}

    float getPowerSum() const {
        return power.r + power.g + power.b;
    }
    std::string toString() const;

};

class Sphere : public Object3D {
public:
    Point center;
    float radius;

    Sphere(const Point& base, const float& radius, const Material& material) : 
        Object3D(material), center(base), radius(radius) {}

    std::optional<Intersection> intersect(const Ray& ray) const;

    std::string toString() const;
};

class Plane : public Object3D {
public:

    Direction normal;
    float distance; // Distance from the origin to the plane along the normal vector

    Plane(const Direction& normal, const Material& material, const float distance = 1) :
        Object3D(material), normal(normal.normalize()), distance(distance) {}

    std::optional<Intersection> intersect(const Ray& ray) const;

    // Returns the distance from the plane to a point
    float distanceTo(const Point& point) const;

    std::string toString() const;
};

class Triangle : public Object3D {
public:
    Point a, b, c;
    Direction normal; // for convenience

    Triangle(const Point& a, const Point& b, const Point& c, const Material& material) :
        Object3D(material), a(a), b(b), c(c), normal((b - a).cross(c - a).normalize()) {}

    std::optional<Intersection> intersect(const Ray& ray) const;

    Point centroid() const {
        return Point((a.x + b.x + c.x) / 3.0f, (a.y + b.y + c.y) / 3.0f, (a.z + b.z + c.z) / 3.0f);
    }

    std::string toString() const;
};

class Cone : public Object3D {
public:
    Point base;
    Direction axis;
    float radius, height;

    Cone(const Point& base, const Direction& axis, float radius, float height, const Material& material) :
        Object3D(material), base(base), axis(axis), radius(radius), height(height) {}

    std::optional<Intersection> intersect(const Ray& ray) const;

    std::string toString() const;
};

class Cylinder : public Object3D {
public:
    Point base;
    Direction axis;
    float radius, height;

    Cylinder(const Point& base, const Direction& axis, float radius, float height, const Material& material) :
        Object3D(material), base(base), axis(axis.normalize()), radius(radius), height(height) {}

    std::optional<Intersection> intersect(const Ray& ray) const;

    std::string toString() const;
};

// Quad: rectangular area light defined by center and two half-edge vectors
class Quad : public Object3D {
public:
    Point center;
    Direction u, v;  // Half-edge vectors
    Direction normal;
    float area_;

    // Constructor takes center point and two half-edge vectors u, v
    // The quad spans from center-u-v to center+u+v
    Quad(const Point& center, const Direction& u, const Direction& v, const Material& material);

    std::optional<Intersection> intersect(const Ray& ray) const;
    
    float area() const override { return area_; }
    std::optional<LightSample> sampleLightPoint() const override;

    std::string toString() const;
};

// Ellipsoid, disk?
