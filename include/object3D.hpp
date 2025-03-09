#pragma once

#include <optional>
#include <vector>
#include <memory>

#include "geometry.hpp"
#include "Image.hpp"
#include "RGB.hpp"

using namespace std;

#define EPSILON 1e-6

static bool const dummy = (srand(time(NULL)), true);

inline float rand0_1() {
  return (float) rand() / (RAND_MAX);
}

struct Intersection {
    float distance;
    Point point;
    Direction normal;
    RGB material;

    Intersection(float distance, const Point& point, const Direction& normal, const RGB& material = RGB(1, 1, 1)) :
        distance(distance), point(point), normal(normal), material(material) {}
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

    RGB material;

    Object3D(const RGB& material = RGB(1, 1, 1)) : material(material) {}

    virtual string toString() const = 0;
    virtual optional<Intersection> intersect(const Ray& ray) const = 0;   
    
};

class Scene {
public:
    vector<shared_ptr<Object3D>> objects;

    void addObject(const shared_ptr<Object3D>& object);
    optional<Intersection> intersect(const Ray& ray) const;
    void sortObjectsByDistanceToCamera(const Point& cameraPosition);
    string toString() const;
};

class PinholeCamera {
public:
 
    PinholeCamera(const Point& origin) : PinholeCamera(origin, 50, 256, 256) {};

    PinholeCamera(const Point& origin, const int FOV, const int width, const int height);

    PinholeCamera(const Point& origin, const Direction& up, const Direction& left, const Direction& forward, int width, int height)
        : origin(origin), left(left.normalize()), up(up.normalize()), forward(forward), width(width), height(height) {}

    Image render(const Scene& scene, unsigned samplesPerPixel) const;

private:

    Point origin;
    Direction left, up, forward;
    int width, height;

    Ray generateRay(float x, float y) const;
    RGB traceRay(const Ray& ray, const Scene& scene) const;
    RGB calculatePixelColor(const Scene& scene, float x, float y, unsigned samplesPerPixel) const;

};

class Sphere : public Object3D {
public:
    Point center;
    float radius;
    float inclinacion, azimut;

    Sphere(const Point& base, const float& radius, const RGB& material = RGB(1, 1, 1)) : 
        Object3D(material), center(base), radius(radius) {}

    optional<Intersection> intersect(const Ray& ray) const;

    string toString() const;
};

class Plane : public Object3D {
public:

    Direction normal;
    int distance;

    Plane(const Direction& normal, const int distance = 1, const RGB& material = RGB(1, 1, 1)) :
        Object3D(material), normal(normal.normalize()), distance(distance) {}

    optional<Intersection> intersect(const Ray& ray) const;

    // Returns the distance from the plane to a point
    float distanceTo(const Point& point) const;

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
