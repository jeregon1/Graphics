#include "../include/object3D.hpp"

/*******
 * Ray *
 *******/

Point Ray::at(float t) const {
    return direction * t + origin;
}

/*********
 * Scene *
 *********/

void Scene::addObject(const shared_ptr<Object3D>& object) {
        objects.push_back(object);
        if (dynamic_pointer_cast<PointLight>(object)) {
            lightIndex.push_back(objects.size()-1);
        }
    }

optional<Intersection> Scene::intersect(const Ray& ray) const {
    optional<Intersection> closest_intersection = nullopt;
    for (const auto& object : objects) {
        auto intersection = object->intersect(ray);
        if (intersection && (!closest_intersection || intersection->distance < closest_intersection->distance)) {
            closest_intersection = intersection;
        }
    }
    return closest_intersection;
}

string Scene::toString() const {
    string result;
    for (const auto& object : objects) {
        result += object->toString() + "\n";
    }
    return result;
}

/***************
 * Point Light *
 ***************/


 
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
Source: https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-sphere-intersection.html
*/
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
    
    /*
    Direction oc = center - r.origin;
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
        return Intersection(t, r.at(t), (r.at(t) - center).normalize(), material);
    }
    */
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

    // If the intersection point is behind the ray origin, there is no intersection
    if (t < 0)
        return nullopt;

    return Intersection(t, r.at(t), normal, material);
}

float Plane::distanceTo(const Point& point) const {
    return normal.dot(point - Point(normal.x, normal.y, normal.z) * distance);
}

string Plane::toString() const {
    ostringstream oss;
    oss << "Base: " << normal*distance << "\n"
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