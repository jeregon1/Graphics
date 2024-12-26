#include <iostream>
#include <cassert>
#include "../include/object3D.hpp"

void testSphereIntersection() {
    Sphere sphere(Point(0, 0, 0), 1);
    Ray ray(Point(0, 0, -3), Direction(0, 0, 1));
    auto intersection = sphere.intersect(ray);
    assert(intersection.has_value());
    assert(abs(intersection->distance - 2.0f) < EPSILON);
    std::cout << "Sphere intersection test passed!" << std::endl;
}

void testPlaneIntersection() {
    Plane plane(Direction(0, 1, 0), 0);
    Ray ray(Point(0, -1, 0), Direction(0, 1, 0));
    auto intersection = plane.intersect(ray);
    assert(intersection.has_value());
    assert(abs(intersection->distance - 1.0f) < EPSILON);
    std::cout << "Plane intersection test passed!" << std::endl;
}

void testTriangleIntersection() {
    Triangle triangle(Point(0, 0, 0), Point(1, 0, 0), Point(0, 1, 0));
    Ray ray(Point(0.25, 0.25, -1), Direction(0, 0, 1));
    auto intersection = triangle.intersect(ray);
    assert(intersection.has_value());
    assert(abs(intersection->distance - 1.0f) < EPSILON);
    std::cout << "Triangle intersection test passed!" << std::endl;
}

void testConeIntersection() {
    Cone cone(Point(0, 0, 0), Direction(0, 1, 0), 1, 2);
    Ray ray(Point(0, 0, -3), Direction(0, 0, 1));
    auto intersection = cone.intersect(ray);
    assert(intersection.has_value());
    assert(abs(intersection->distance - 2.0f) < EPSILON);
    std::cout << "Cone intersection test passed!" << std::endl;
}

int main() {
    testSphereIntersection();
    testPlaneIntersection();
    testTriangleIntersection();
    testConeIntersection();
    std::cout << "All intersection tests passed!" << std::endl;
    return 0;
}
