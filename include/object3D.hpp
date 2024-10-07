#pragma once

#include <optional>

#include "geometry.hpp"

using namespace std;

#define EPSILON 1e-6

struct Intersection {
    float distance;
    Point point;
    Direction normal;

    Intersection(float distance, const Point& point, const Direction& normal):
        distance(distance), point(point), normal(normal) {}
};

class Ray {
public:
    Point origin;
    Direction direction;

    Ray(const Point& origin, const Direction& direction):
        origin(origin), direction(direction) {}

    // Returns the point at a distance t from the origin
    Point at(float t) const;
};

class Object3D {
public:
    virtual string toString() const = 0;

    virtual optional<Intersection> intersect(const Ray& ray) const = 0;    
};

class Sphere : public Object3D {
public:
    Point center;
    float radius;
    float inclinacion, azimut;

    Sphere(const Point& base, const float& radius) : 
        center(base), radius(radius) {}

    optional<Intersection> intersect(const Ray& ray) const;

    string toString() const;
};

class Plane : public Object3D {
public:
    Point base;
    Direction normal;

    Plane(const Point& base, const Direction& normal) :
        base(base), normal(normal) {}

    optional<Intersection> intersect(const Ray& ray) const;

    // Returns the distance from the plane to a point
    float distance(const Point& point) const;

    string toString() const;
};

class Triangle : public Object3D {
public:
    Point a, b, c;
    Direction normal;

    Triangle(const Point& a, const Point& b, const Point& c) :
        a(a), b(b), c(c), normal((b - a).cross(c - a).normalize()) {}

    optional<Intersection> intersect(const Ray& ray) const;

    string toString() const;
};

class Cone : public Object3D {
public:
    Point base;
    Direction axis;
    float radius, height;

    Cone(const Point& base, const Direction& axis, float radius, float height) :
        base(base), axis(axis), radius(radius), height(height) {}

    optional<Intersection> intersect(const Ray& ray) const;

    string toString() const;
};

// Cylinder, ellipsoid, disk
