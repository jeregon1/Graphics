#include "constants.hpp"
#include "../include/object3D.hpp"

#include <vector>
#include <memory>
#include <optional>
#include <list>
#include <string>
#include <sstream>
#include <fstream>
#include <algorithm>

using namespace std;

/*******
 * Ray *
 *******/

Point Ray::at(float t) const {
    return direction * t + origin;
}

/*************
 * PointLight *
 *************/

string PointLight::toString() const {
    ostringstream oss;
    oss << "- PointLight:\n"
        << "  Center: " << center << "\n"
        << "  Light: RGB(" << light.r << ", " << light.g << ", " << light.b << ")";
    return oss.str();
}
 
/**********
 * Sphere *
 **********/

string Sphere::toString() const {
    ostringstream oss;
    oss << "- Sphere:\n"
        << "  Center: " << center << "\n"
        << "  Radius: " << radius << "\n"
        << "  " << material.toString();
    return oss.str();
}

// Source: https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-sphere-intersection.html
optional<Intersection> Sphere::intersect(const Ray& r) const {
    
    Direction oc = center - r.origin;
    float tca = oc.dot(r.direction);

    if (tca < 0) {
        return nullopt;
    }

    float d2 = oc.dot(oc) - tca * tca;

    if (d2 > radius * radius) { 
        return nullopt;
    }

    float thc = sqrt(radius * radius - d2);
    float t0 = tca - thc;
    float t1 = tca + thc;

    if (t0 < 0 && t1 < 0) {
        return nullopt;
    }

    float t = (t0 < t1) ? t0 : t1;

    if (t < 0) {
        t = (t0 > t1) ? t0 : t1;
        if (t < 0) {
            return nullopt;
        }
    }

    return Intersection(t, r.at(t), (r.at(t) - center).normalize(), material);
}


/*********
 * Plane *
 *********/

// Source: https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-plane-and-ray-disk-intersection.html
optional<Intersection> Plane::intersect(const Ray& r) const {
    float denominator = normal.dot(r.direction);
    
    // If the ray is parallel to the plane, there is no intersection
    if (abs(denominator) < EPSILON)
        return nullopt;

    Point base = Point(normal.x, normal.y, normal.z) * distance; // Point of the plane
    float t = normal.dot(base - r.origin) / denominator; // Distance from the ray origin to the intersection point

    // If the intersection point is behind the ray origin, it means there's no intersection
    if (t < 0)
        return nullopt;

    return Intersection(t, r.at(t), normal, material);
}

float Plane::distanceTo(const Point& point) const {
    return normal.dot(point - Point(normal.x, normal.y, normal.z) * distance);
}

string Plane::toString() const {
    ostringstream oss;
    oss << "- Plane:\n"
        << "  Base: " << normal*distance << "\n"
        << "  Normal: " << normal << "\n"
        << "  " << material.toString();
    return oss.str();
}

/************
 * Triangle *
 ************/

// Möller-Trumbore intersection algorithm
optional<Intersection> Triangle::intersect(const Ray& r) const {
    const float EPS = 1e-8f;
    
    Direction edge1 = b - a;
    Direction edge2 = c - a;
    Direction h = r.direction.cross(edge2);
    float a_det = edge1.dot(h);

    // Check if the ray is parallel to the triangle
    if (abs(a_det) < EPS)
        return nullopt;

    float f = 1.0f / a_det;
    Direction s = r.origin - a;
    float u = f * s.dot(h);

    // Check if the intersection point is outside the triangle
    if (u < 0.0f || u > 1.0f)
        return nullopt;

    Direction q = s.cross(edge1);
    float v = f * r.direction.dot(q);

    // Check if the intersection point is outside the triangle
    if (v < 0.0f || u + v > 1.0f)
        return nullopt;

    float t = f * edge2.dot(q);

    // Check if the intersection point is in front of the ray origin
    if (t > EPS) {
        Point intersectionPoint = r.at(t);
        return Intersection(t, intersectionPoint, normal, material);
    } else {
        return nullopt;
    }
}

string Triangle::toString() const {
    ostringstream oss;
    oss << "- Triangle:\n"
        << "  A: " << a << "\n"
        << "  B: " << b << "\n"
        << "  C: " << c << "\n"
        << "  " << material.toString();
    return oss.str();
}

/********
 * Cone *
 ********/

optional<Intersection> Cone::intersect(const Ray& ray) const {
    const float EPS = 1e-8f;
    
    // Transform ray to cone's local coordinate system
    // Assume cone axis is along positive Y, with tip at (0, height, 0)
    Direction co = ray.origin - base;
    
    // Cone equation: x² + z² = (radius * (height - y) / height)²
    // Rearranging: x² + z² - (radius² * (height - y)² / height²) = 0
    
    float k = radius / height;
    k = k * k;
    
    float a = ray.direction.x * ray.direction.x + ray.direction.z * ray.direction.z - k * ray.direction.y * ray.direction.y;
    float b = 2.0f * (co.x * ray.direction.x + co.z * ray.direction.z - k * (height - co.y) * (-ray.direction.y));
    float c = co.x * co.x + co.z * co.z - k * (height - co.y) * (height - co.y);

    float discriminant = b * b - 4.0f * a * c;

    if (discriminant < 0) {
        return nullopt;
    }

    float sqrt_discriminant = sqrt(discriminant);
    float t1 = (-b - sqrt_discriminant) / (2.0f * a);
    float t2 = (-b + sqrt_discriminant) / (2.0f * a);

    // Choose the closest positive intersection
    float t = (t1 > EPS) ? t1 : t2;
    if (t <= EPS) {
        return nullopt;
    }

    Point intersectionPoint = ray.at(t);
    Direction localPoint = intersectionPoint - base;
    
    // Check if intersection is within cone bounds (0 <= y <= height)
    if (localPoint.y < 0 || localPoint.y > height) {
        return nullopt;
    }

    // Calculate normal at intersection point
    // For a cone, normal at (x, y, z) is (x, -radius²/height, z) normalized
    float normalY = -radius * radius / height;
    Direction normal = Direction(localPoint.x, normalY, localPoint.z).normalize();
    
    return Intersection(t, intersectionPoint, normal, material);
}

string Cone::toString() const {
    ostringstream oss;
    oss << "- Cone:\n"
        << "  Base: " << base << "\n"
        << "  Axis: " << axis << "\n"
        << "  Radius: " << radius << "\n"
        << "  Height: " << height << "\n"
        << "  " << material.toString();
    return oss.str();
}

/*************
 * Cylinder *
 *************/

optional<Intersection> Cylinder::intersect(const Ray& ray) const {
    const float EPS = 1e-8f;
    
    // Transform ray to cylinder's local coordinate system
    // Assume cylinder axis is along Y direction
    Direction co = ray.origin - base;
    
    // For a cylinder with axis along Y, equation is: x² + z² = radius²
    // We only consider the x and z components for the cylindrical surface
    
    float a = ray.direction.x * ray.direction.x + ray.direction.z * ray.direction.z;
    
    // If ray is parallel to cylinder axis, no intersection with curved surface
    if (abs(a) < EPS) {
        return nullopt;
    }
    
    float b = 2.0f * (co.x * ray.direction.x + co.z * ray.direction.z);
    float c = co.x * co.x + co.z * co.z - radius * radius;

    float discriminant = b * b - 4.0f * a * c;

    if (discriminant < 0) {
        return nullopt;
    }

    float sqrt_discriminant = sqrt(discriminant);
    float t1 = (-b - sqrt_discriminant) / (2.0f * a);
    float t2 = (-b + sqrt_discriminant) / (2.0f * a);

    // Check both intersections and choose the closest valid one
    float t = -1;
    
    // Check first intersection
    if (t1 > EPS) {
        Point testPoint = ray.at(t1);
        Direction localPoint = testPoint - base;
        if (localPoint.y >= 0 && localPoint.y <= height) {
            t = t1;
        }
    }
    
    // Check second intersection if first wasn't valid
    if (t < 0 && t2 > EPS) {
        Point testPoint = ray.at(t2);
        Direction localPoint = testPoint - base;
        if (localPoint.y >= 0 && localPoint.y <= height) {
            t = t2;
        }
    }
    
    if (t <= EPS) {
        return nullopt;
    }

    Point intersectionPoint = ray.at(t);
    Direction localPoint = intersectionPoint - base;
    
    // Calculate normal at intersection point
    // For a cylinder, normal is perpendicular to the axis and pointing outward
    Direction normal = Direction(localPoint.x, 0, localPoint.z).normalize();
    
    return Intersection(t, intersectionPoint, normal, material);
}

string Cylinder::toString() const {
    ostringstream oss;
    oss << "- Cylinder:\n"
        << "  Base: " << base << "\n"
        << "  Axis: " << axis << "\n"
        << "  Radius: " << radius << "\n"
        << "  Height: " << height << "\n"
        << "  " << material.toString();
    return oss.str();
}