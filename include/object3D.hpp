#pragma once

#include <optional>
#include <vector>
#include <memory>
#include <string>

#include "geometry.hpp"
#include "Image.hpp"
#include "RGB.hpp"
#include "foton.hpp"
#include "kernel.hpp"
#include "utils.hpp"

#define EPSILON 1e-6

static bool const dummy = (srand(time(NULL)), true);

inline float rand0_1() {
  return (float) rand() / (RAND_MAX);
}

struct Material {
    RGB diffuse; // Color difuso
    RGB specular; // Color especular
    RGB transparency; // Color de transparencia (no se usa en este proyecto)
    double p_diffuse = 0.0; // Probabilidad de difuso
    double p_specular = 0.0; // Probabilidad de especular
    double p_transparency = 0.0; // Probabilidad de refracción
    double n = 1.0; // Índice de refracción (no se usa en este proyecto)
    bool isEmissive = false; // Si es una fuente de luz

    Material(const RGB& diffuse = RGB(0, 0, 0), const RGB& specular = RGB(0, 0, 0), bool isEmissive = false) :
        diffuse(diffuse), specular(specular), isEmissive(isEmissive) 
        {
            p_diffuse = diffuse.max();
            p_specular = specular.max();
            p_transparency = transparency.max();

            double totalProbability = p_diffuse + p_specular + p_transparency;
            if (totalProbability > 0.0) {
                p_diffuse = 0.9 * p_diffuse / totalProbability;
                p_specular = 0.9 * p_specular / totalProbability;
                p_transparency = 0.9 * p_transparency / totalProbability;
            }
        }

    Direction refractar(const Direction& wo, const Direction& normal) const {
        // Implementación de la refracción usando la ley de Snell
        float n1 = 1.0f; // Índice de refracción del aire
        float n2 = n; // Índice de refracción del material 
        float cosThetaI = -normal.dot(wo); // TODO: Porque hay un signo -
        float sinThetaT2 = (n1 / n2) * (n1 / n2) * (1.0f - cosThetaI * cosThetaI);
        
        if (sinThetaT2 > 1.0f) {
            return Direction(0, 0, 0); // Total internal reflection
        }
        
        float cosThetaT = sqrt(1.0f - sinThetaT2);
        return (wo * (n1 / n2) + normal * (n1 / n2 * cosThetaI - cosThetaT)).normalize();
    }

};

struct Intersection {
    float distance;
    Point point;
    Direction normal;
    Material material;

    Intersection(const float distance, const Point& point, const Direction& normal, const Material& material) :
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

    Material material;

    Object3D(const Material& material) : material(material) {}

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
    MapaFotones generarMapaFotones(int nPaths, bool save, double sigma = 0.0f);
    void reboteFoton(const Ray& ray, const RGB& light, std::list<Foton>& fotones, std::list<Foton>& causticos, bool esCaustico, bool save = false, double sigma = 0.0f);
    RGB ecuacionRenderFotones(Point x, Direction wo, Object3D geo, Direction n, MapaFotones mapa, int kFotones, double radio, bool guardar, Kernel* kernel, double sigma = 0.0f);
    RGB estimacionSiguienteEvento(Point x, Direction wo, Object3D g, Direction n, double sigma = 0.0f);

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
    Image renderPhotonMapping(const Scene& scene, unsigned samplesPerPixel, 
                MapaFotones mapa, unsigned kFotones, double radio, Kernel* kernel) const;

private:

    Point origin;
    Direction left, up, forward;
    int width, height;

    Ray generateRay(float x, float y) const;
    RGB traceRay(const Ray& ray, const Scene& scene) const;
    RGB tracePath(const Ray& ray, const Scene& scene, unsigned depth = 0) const;
    RGB calculatePixelColorRayTracing(const Scene& scene, float x, float y, unsigned samplesPerPixel) const;
    RGB calculatePixelColorPathTracing(const Scene& scene, float x, float y, unsigned samplesPerPixel) const;
    RGB calculatePixelColorPhotonMapping(const Scene& scene, float x, float y, unsigned samplesPerPixel) const;

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

    Plane(const Direction& normal, const Material& material, const int distance = 1) :
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

    Triangle(const Point& a, const Point& b, const Point& c, const Material& material) :
        Object3D(material), a(a), b(b), c(c), normal((b - a).cross(c - a).normalize()) {}

    std::optional<Intersection> intersect(const Ray& ray) const;

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

// Cylinder, ellipsoid, disk
