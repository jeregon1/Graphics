
#include "../include/object3D.hpp"


/*******
 * Ray *
 *******/

Point Ray::at(float t) const {
    return direction * t + origin;
}

/**********
 * Sphere *
 **********/

string Sphere::toString() const {
    ostringstream oss;
    oss << "Center: " << center << "\n"
        << "Radius: " << radius;
    return oss.str();
}

/*
Implicit equation: f(x, y, z) = (x - c.x)^2 + (y - c.y)^2 + (z - c.z)^2 - r^2
Source: https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-sphere-intersection.html
*/
optional<Intersection> Sphere::intersect(const Ray& r) const {
    Direction oc = r.origin - center;
    float a = r.direction.dot(r.direction);
    float b = 2.0f * oc.dot(r.direction);
    float c = oc.dot(oc) - radius * radius;
    float discriminant = b * b - 4 * a * c;

    if (discriminant < 0) {
        return nullopt;
    } else {
        float t1 = (-b - sqrt(discriminant)) / (2.0f * a);
        float t2 = (-b + sqrt(discriminant)) / (2.0f * a);
        
        // If both intersection points are behind the ray origin, no intersection
        if (t1 < 0 && t2 < 0) return nullopt;

        float t;
        // If one intersection point is behind the ray origin, return the other
        if (t1 < 0) t = t2;
        if (t2 < 0) t = t1;

        // Return the closest intersection point
        t = min(t1, t2);
        return Intersection(t, r.at(t), (r.at(t) - center).normalize());
    }
}

/*********
 * Plane *
 *********/

// Source: https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-plane-and-ray-disk-intersection.html
optional<Intersection> Plane::intersect(const Ray& r) const {
    float denominator = normal.dot(r.direction);

    // If the ray is parallel to the plane, there is no intersection
    if (denominator < EPSILON)
        return nullopt;

    float t = normal.dot(base - r.origin) / denominator;

    // If the intersection point is behind the ray origin, there is no intersection
    if (t < 0)
        return nullopt;

    return Intersection(t, r.at(t), normal);
}

float Plane::distance(const Point& point) const {
    return normal.dot(point - base);
}

string Plane::toString() const {
    ostringstream oss;
    oss << "Base: " << base << "\n"
        << "Normal: " << normal;
    return oss.str();
}

/************
 * Triangle *
 ************/

// TODO: check
optional<Intersection> Triangle::intersect(const Ray& r) const {
    // Source: https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm
    Direction edge1 = b - a;
    Direction edge2 = c - a;
    Direction h = r.direction.cross(edge2);
    float a_ = edge1.dot(h);

    // Check if the ray is parallel to the triangle
    if (abs(a_) < EPSILON)
        return nullopt;

    float f = 1.0f / a_; // Inverse of the determinant
    Direction s = r.origin - a;
    float u = f * s.dot(h);

    // Check if the intersection point is outside the triangle
    if (u < 0.0f || 1.0f < u)
        return nullopt;

    Direction q = s.cross(edge1); 
    float v = f * r.direction.dot(q);

    // Check if the intersection point is outside the triangle
    if (v < 0.0f || u + v > 1.0f)
        return nullopt;

    float t = f * edge2.dot(q);

    // Check if the intersection point is in front of the ray origin
    if (t > EPSILON)
        return Intersection(t, r.at(t), normal);
    else
        return nullopt;
}

string Triangle::toString() const {
    ostringstream oss;
    oss << "A: " << a << "\n"
        << "B: " << b << "\n"
        << "C: " << c;
    return oss.str();
}

/********
 * Cone *
 ********/

// Source: https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-cone-intersection
optional<Intersection> Cone::intersect(const Ray& ray) const {
    Direction co = ray.origin - base;
    float cos2 = (radius / height) * (radius / height);
    float a = ray.direction.x * ray.direction.x + ray.direction.y * ray.direction.y - cos2 * ray.direction.z * ray.direction.z;
    float b = 2 * (ray.direction.x * co.x + ray.direction.y * co.y - cos2 * ray.direction.z * co.z);
    float c = co.x * co.x + co.y * co.y - cos2 * co.z * co.z;

    float discriminant = b * b - 4 * a * c;

    if (discriminant < 0) {
        return nullopt;
    } else {
        float t1 = (-b - sqrt(discriminant)) / (2 * a);
        float t2 = (-b + sqrt(discriminant)) / (2 * a);

        float t = (t1 < t2) ? t1 : t2;

        if (t < 0) {
            t = (t1 > t2) ? t1 : t2;
            if (t < 0) {
                return nullopt;
            }
        }

        Point intersectionPoint = ray.at(t);
        Direction normal = (intersectionPoint - base).normalize();

        return Intersection(t, intersectionPoint, normal);
    }
}

string Cone::toString() const {
    stringstream ss;
    ss << "Cone(base: " << base.toString() << ", axis: " << axis.toString() << ", radius: " << radius << ", height: " << height << ")";
    return ss.str();
}

/*************
 * Cylinder *
 *************/