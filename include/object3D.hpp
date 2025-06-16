#pragma once

#include <optional>
#include <string>
#include <fstream>

#include "geometry.hpp"
#include "RGB.hpp"


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

    std::string toString() const {
        std::ostringstream oss;
        oss << "Material(diffuse: " << diffuse << ", specular: " << specular << ", transparency: " << transparency 
            << ", isEmissive: " << (isEmissive ? "true" : "false") << ")";
        return oss.str();
    }
    
    // Equality operator for comparing materials
    bool operator==(const Material& other) const {
        return diffuse == other.diffuse && specular == other.specular && transparency == other.transparency && isEmissive == other.isEmissive;
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
    RGB light;

    PointLight(const Point& center, const RGB& emission) : center(center), light(emission) {}

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
