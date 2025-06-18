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
        << "  Light: RGB(" << power.r << ", " << power.g << ", " << power.b << ")";
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

    if (tca < 0) 
        return nullopt;

    float d2 = oc.dot(oc) - tca * tca;

    if (d2 > radius * radius)
        return nullopt;

    float thc = sqrt(radius * radius - d2);
    float t0 = tca - thc;
    float t1 = tca + thc;

    if (t0 < 0 && t1 < 0)
        return nullopt;

    float t = std::min(t0, t1);

    if (t < 0) {
        t = std::max(t0, t1);
        if (t < 0)
            return nullopt;
    }

    Point hitPoint = r.at(t);
    Direction outwardNormal = (hitPoint - center).normalize();
    
    // For refraction to work correctly, normal should point toward the incoming ray
    // If ray is inside sphere (hitting from inside), flip the normal
    bool rayFromInside = (r.origin - center).mod() < radius;
    Direction normal = rayFromInside ? -outwardNormal : outwardNormal;

    return Intersection(t, hitPoint, normal, material);
}


/*********
 * Plane *
 *********/

// Source: https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-plane-and-ray-disk-intersection.html
optional<Intersection> Plane::intersect(const Ray& r) const {
    float denominator = normal.dot(r.direction);
    
    // If the ray is parallel to the plane, there is no intersection
    if (abs(denominator) < EPS)
        return nullopt;

    Point base = Point(normal.x, normal.y, normal.z) * distance; // Point of the plane
    float t = normal.dot(base - r.origin) / denominator; // Distance from the ray origin to the intersection point

    // If the intersection point is behind the ray origin, it means there's no intersection
    if (t < 0)
        return nullopt;

    // Flip normal if ray hits from behind (ensure normal faces toward incoming ray)
    Direction hitNormal = (denominator < 0) ? normal : -normal;

    return Intersection(t, r.at(t), hitNormal, material);
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
    
    // 1) Build orthonormal basis (u,v,w) where w = cone axis
    Direction w = axis.normalize();
    Direction tmp = fabs(w.x) < 0.9f ? Direction(1,0,0) : Direction(0,1,0);
    Direction u = w.cross(tmp).normalize();
    Direction v = w.cross(u);

    // 2) Transform ray into cone-local coordinates
    Direction orig = ray.origin - base;
    Point lo(orig.dot(u), orig.dot(w), orig.dot(v));
    Direction ld(ray.direction.dot(u), ray.direction.dot(w), ray.direction.dot(v));

    // 3) Intersect with base cap (plane y=0)
    float t_base = -1.0f;
    if (fabs(ld.y) > EPS) {
        float t = -lo.y / ld.y;
        if (t > EPS) {
            Point p_base = lo + ld * t;
            if (p_base.x * p_base.x + p_base.z * p_base.z <= radius * radius) {
                t_base = t;
            }
        }
    }

    // 4) Intersect with curved surface
    float k2 = (radius/height)*(radius/height);
    float a = ld.x*ld.x + ld.z*ld.z - k2*ld.y*ld.y;
    float b = 2.0f*(lo.x*ld.x + lo.z*ld.z + k2*(height - lo.y)*ld.y);
    float c = lo.x*lo.x + lo.z*lo.z - k2*(height - lo.y)*(height - lo.y);

    float disc = b*b - 4.0f*a*c;
    float t_side = -1.0f;
    if (disc >= 0) {
        float sd = sqrt(disc);
        float t0 = (-b - sd)/(2.0f*a);
        float t1 = (-b + sd)/(2.0f*a);

        // Check both intersection points
        for (float t_cand : {t0, t1}) {
            if (t_cand > EPS) {
                float y = lo.y + ld.y * t_cand;
                if (y >= 0 && y <= height) {
                    if (t_side < 0 || t_cand < t_side) {
                        t_side = t_cand;
                    }
                }
            }
        }
    }

    // 5) Find nearest valid intersection
    float t_hit = -1.0f;
    Direction N_world;

    if (t_base > EPS && (t_side < 0 || t_base < t_side)) {
        t_hit = t_base;
        N_world = -w; // Normal of base cap points away from cone
    } else if (t_side > EPS) {
        t_hit = t_side;
        // Normal for curved surface
        Point lp = lo + ld * t_hit;
        Direction N_local = Direction(lp.x, k2 * (height - lp.y), lp.z).normalize();
        // Transform normal back to world coordinates
        N_world = u * N_local.x + w * N_local.y + v * N_local.z;
    }

    if (t_hit > EPS) {
        Point P_world = ray.at(t_hit);
        Direction normal = N_world;
        if (ray.direction.dot(normal) > 0) normal = -normal;
        return Intersection(t_hit, P_world, normal, material);
    }

    return nullopt;
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

    // 1) Build orthonormal basis (u,v,w) where w is the cylinder axis
    Direction w = axis.normalize();
    Direction tmp = (fabs(w.x) < 0.9f) ? Direction(1, 0, 0) : Direction(0, 1, 0);
    Direction u = w.cross(tmp).normalize();
    Direction v = w.cross(u);

    // 2) Transform ray into local coordinates
    Direction orig = ray.origin - base;
    Point lo(orig.dot(u), orig.dot(w), orig.dot(v));
    Direction ld(ray.direction.dot(u), ray.direction.dot(w), ray.direction.dot(v));

    // 3) Intersection with caps
    float t_base = -1.0f, t_top = -1.0f;
    if (fabs(ld.y) > EPS) { // Ray is not parallel to the caps
        // Base cap (y=0)
        float t0 = -lo.y / ld.y;
        if (t0 > EPS) {
            Point p_base = lo + ld * t0;
            if (p_base.x * p_base.x + p_base.z * p_base.z <= radius * radius) {
                t_base = t0;
            }
        }
        // Top cap (y=height)
        float t1 = (height - lo.y) / ld.y;
        if (t1 > EPS) {
            Point p_top = lo + ld * t1;
            if (p_top.x * p_top.x + p_top.z * p_top.z <= radius * radius) {
                t_top = t1;
            }
        }
    }

    // 4) Intersection with curved surface (x^2 + z^2 = r^2)
    float a = ld.x * ld.x + ld.z * ld.z;
    float b = 2.0f * (lo.x * ld.x + lo.z * ld.z);
    float c = lo.x * lo.x + lo.z * lo.z - radius * radius;
    float disc = b * b - 4.0f * a * c;
    float t_side = -1.0f;

    if (disc >= 0) {
        float sd = sqrt(disc);
        float t0 = (-b - sd) / (2.0f * a);
        float t1 = (-b + sd) / (2.0f * a);

        for (float t_cand : {t0, t1}) {
            if (t_cand > EPS) {
                float y = lo.y + ld.y * t_cand;
                if (y >= 0 && y <= height) {
                    if (t_side < 0 || t_cand < t_side) {
                        t_side = t_cand;
                    }
                }
            }
        }
    }

    // 5) Find the nearest valid intersection
    float t_min = -1.0f;
    if (t_base > EPS) t_min = t_base;
    if (t_top > EPS && (t_min < 0 || t_top < t_min)) t_min = t_top;
    if (t_side > EPS && (t_min < 0 || t_side < t_min)) t_min = t_side;

    if (t_min < 0) return nullopt;

    // 6) Determine normal and return intersection
    Direction N_world;
    if (t_min == t_base) {
        N_world = -w;
    } else if (t_min == t_top) {
        N_world = w;
    } else { // t_min == t_side
        Point p_local = lo + ld * t_min;
        Direction N_local = Direction(p_local.x, 0, p_local.z).normalize();
        N_world = u * N_local.x + w * N_local.y + v * N_local.z;
    }
    
    Point P_world = ray.at(t_min);
    Direction normal = N_world;
    if (ray.direction.dot(normal) > 0) normal = -normal;
    return Intersection(t_min, P_world, normal, material);
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
