#pragma once

#include <optional>
#include <vector>
#include <memory>
#include <string>
#include <list>
#include <fstream>
#include <sstream>

#include "geometry.hpp"
#include "RGB.hpp"
#include "foton.hpp"
#include "kernel.hpp"
#include "utils.hpp"


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
        diffuse(diffuse), specular(specular), transparency(RGB(0, 0, 0)), isEmissive(isEmissive) 
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

    PointLight(const Point& center, const RGB& emission) : center(center), light(emission) {}

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
    MapaFotones generarMapaFotones(int nPaths, bool save, double sigma = 0.0f) const;
    void reboteFoton(const Ray& ray, const RGB& light, std::list<Foton>& fotones, std::list<Foton>& causticos, bool esCaustico, bool save = false, double sigma = 0.0f) const;
    RGB ecuacionRenderFotones(Point x, Direction wo, Material material, Direction n, MapaFotones mapa, int kFotones, double radio, bool guardar, Kernel* kernel, double sigma = 0.0f) const;
    RGB estimacionSiguienteEvento(Point point, Direction wo, Material material, Direction n, double sigma) const;
 
    void sortObjectsByDistanceToCamera(const Point& cameraPosition); // No implementado
    std::string toString() const;

    // Simple YAML-like scene loader (no external libs)
    static Scene fromYAML(const std::string& filename); // Declaration only
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

// Ellipsoid, disk

// Implementation of Scene::fromYAML
inline Scene Scene::fromYAML(const std::string& filename) {
    Scene scene;
    std::ifstream file(filename);
    std::string line;
    Material currentMaterial;
    while (std::getline(file, line)) {
        // Trim whitespace
        size_t first = line.find_first_not_of(" \\t\\r\\n");
        if (first == std::string::npos) continue; // skip empty/whitespace lines
        line = line.substr(first);
        if (line.empty()) continue;
        std::istringstream iss(line);
        std::string keyword;
        iss >> keyword;
        if (keyword == "background:") {
            float r, g, b;
            iss >> r >> g >> b;
            scene.backgroundColor = RGB(r, g, b);
        } else if (keyword == "material:") {
            float r, g, b;
            iss >> r >> g >> b;
            currentMaterial = Material(RGB(r, g, b), RGB(0,0,0));
        } else if (keyword == "sphere:") {
            float x, y, z, radius;
            iss >> x >> y >> z >> radius;
            scene.addObject(std::make_shared<Sphere>(Point(x, y, z), radius, currentMaterial));
        } else if (keyword == "plane:") {
            float nx, ny, nz, d;
            iss >> nx >> ny >> nz >> d;
            scene.addObject(std::make_shared<Plane>(Direction(nx, ny, nz), currentMaterial, (int)d));
        } else if (keyword == "light:") {
            float x, y, z, r, g, b;
            iss >> x >> y >> z >> r >> g >> b;
            scene.addLight(std::make_shared<PointLight>(Point(x, y, z), RGB(r, g, b)));
        }
    }
    return scene;
}
