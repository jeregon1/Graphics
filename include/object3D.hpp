#pragma once

#include <optional>
#include <vector>
#include <memory>
#include <string>

#include "geometry.hpp"
#include "Image.hpp"
#include "RGB.hpp"

#define EPSILON 1e-6

static bool const dummy = (srand(time(NULL)), true);

inline float rand0_1() {
  return (float) rand() / (RAND_MAX);
}

struct Intersection {
    float distance;
    Point point;
    Direction normal;
    Material material;

    Intersection(float distance, const Point& point, const Direction& normal, const Material& material = Material()) :
        distance(distance), point(point), normal(normal), material(material) {}
};

// TODO: Igual hay que añadir algo más en Material?
struct Material {
    RGB diffuse; // Color difuso
    RGB specular; // Color especular
    RGB glossy; // Color brillante
    float shininess; // Exponente de brillo ??

    Material(const RGB& diffuse = RGB(0, 0, 0), const RGB& specular = RGB(0, 0, 0), const RGB& glossy = RGB(0, 0, 0), float shininess = 32.0f) :
        diffuse(diffuse), specular(specular), glossy(glossy), shininess(shininess) {}
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

    Object3D(const Material& material = Material()) : material(material) {}

    virtual std::string toString() const = 0;
    virtual std::optional<Intersection> intersect(const Ray& ray) const = 0;   
};

class PointLight {
public:

    Point center;
    RGB light;

    // Hay que sobreescribir el material porque es el valor de emisión y no puede ser (1, 1, 1)
    PointLight(const Point& center, const RGB& material) : center(center), light(material) {}

    std::string toString() const;

};

class Scene {
public:

    Scene() = default;
    Scene(const RGB& backgroundColor) : backgroundColor(backgroundColor) {}

    std::vector<std::shared_ptr<Object3D>> objects;
    std::vector<std::shared_ptr<PointLight>> lights;
    RGB backgroundColor = RGB(0, 0, 0); // Color de fondo por defecto

    void addObject(const std::shared_ptr<Object3D>& object);
    void addLight(const std::shared_ptr<PointLight>& light);
    std::optional<Intersection> intersect(const Ray& ray, const float distance = 1000.0f) const;
    
    RGB calculateDirectLight(const Point& p) const;
    
    void sortObjectsByDistanceToCamera(const Point& cameraPosition); // No implementado
    std::string toString() const;
};

class PinholeCamera {
public:
 
    PinholeCamera(const Point& origin) : PinholeCamera(origin, 50, 256, 256) {};

    PinholeCamera(const Point& origin, const int FOV, const int width, const int height);

    PinholeCamera(const Point& origin, const Direction& up, const Direction& left, const Direction& forward, int width, int height)
        : origin(origin), left(left.normalize()), up(up.normalize()), forward(forward), width(width), height(height) {}

    Image renderRayTracing(const Scene& scene, unsigned samplesPerPixel) const;
    Image renderPathTracing(const Scene& scene, unsigned samplesPerPixel) const;

private:

    Point origin;
    Direction left, up, forward;
    int width, height;

    Ray generateRay(float x, float y) const;
    RGB traceRay(const Ray& ray, const Scene& scene) const;
    RGB tracePath(const Ray& ray, const Scene& scene, unsigned depth = 0) const;
    RGB calculatePixelColorRayTracing(const Scene& scene, float x, float y, unsigned samplesPerPixel) const;
    RGB calculatePixelColorPathTracing(const Scene& scene, float x, float y, unsigned samplesPerPixel) const;
};

class Sphere : public Object3D {
public:
    Point center;
    float radius;
    float inclinacion, azimut;

    Sphere(const Point& base, const float& radius, const Material& material) : 
        Object3D(material), center(base), radius(radius) {}

    std::optional<Intersection> intersect(const Ray& ray) const;

    std::string toString() const;
};

class Plane : public Object3D {
public:

    Direction normal;
    int distance;

    Plane(const Direction& normal, const int distance = 1, const Material& material) :
        Object3D(material), normal(normal.normalize()), distance(distance) {}

    std::optional<Intersection> intersect(const Ray& ray) const;

    // Returns the distance from the plane to a point
    float distanceTo(const Point& point) const;

    std::string toString() const;
};

// No implementados
class Triangle : public Object3D {
public:
    Point a, b, c;
    Direction normal;

    Triangle(const Point& a, const Point& b, const Point& c) :
        a(a), b(b), c(c), normal((b - a).cross(c - a).normalize()) {}

    std::optional<Intersection> intersect(const Ray& ray) const;

    std::string toString() const;
};

class Cone : public Object3D {
public:
    Point base;
    Direction axis;
    float radius, height;

    Cone(const Point& base, const Direction& axis, float radius, float height) :
        base(base), axis(axis), radius(radius), height(height) {}

    std::optional<Intersection> intersect(const Ray& ray) const;

    std::string toString() const;
};

// Cylinder, ellipsoid, disk
