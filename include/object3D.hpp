#pragma once

#include <optional>
#include <string>
#include <fstream>
#include <cmath>

#include "geometry.hpp"
#include "RGB.hpp"
#include "constants.hpp"
#include "material.hpp"


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
    
    // Accessor for material
    const Material& getMaterial() const { return material; }

    virtual std::string toString() const = 0;
    friend std::ostream& operator<<(std::ostream& os, const Object3D& obj) {
        os << obj.toString();
        return os;
    }
};

class PointLight {
public:

    Point center;
    RGB power;

    PointLight(const Point& center, const RGB& emission) : center(center), power(emission) {}

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
    int distance; // Distance from the origin to the plane along the normal vector

    Plane(const Direction& normal, const Material& material, const int distance = 1) :
        Object3D(material), normal(normal.normalize()), distance(distance) {}

    std::optional<Intersection> intersect(const Ray& ray) const;

    // Returns the distance from the plane to a point
    float distanceTo(const Point& point) const;

    std::string toString() const;
};

class Triangle : public Object3D {
public:
    Point a, b, c;
    Direction normal;

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

// Ellipsoid, disk?
