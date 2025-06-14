#include <iostream>
#include <cassert>
#include "../include/object3D.hpp"
#include "../include/constants.hpp"

void test_sphere_intersection() {
    Sphere sphere(Point(0, 0, 0), 1, Material());
    Ray ray(Point(0, 0, -3), Direction(0, 0, 1));
    auto intersection = sphere.intersect(ray);
    assert(intersection.has_value());
    assert(abs(intersection->distance - 2.0f) < EPSILON);
    std::cout << "Sphere intersection test passed!" << std::endl;
}

void test_plane_intersection() {
    Plane plane(Direction(0, 1, 0), Material(), 0);
    Ray ray(Point(0, -1, 0), Direction(0, 1, 0));
    auto intersection = plane.intersect(ray);
    assert(intersection.has_value());
    assert(abs(intersection->distance - 1.0f) < EPSILON);
    std::cout << "Plane intersection test passed!" << std::endl;
}

void test_triangle_intersection() {
    Triangle triangle(Point(0, 0, 0), Point(1, 0, 0), Point(0, 1, 0), Material());
    Ray ray(Point(0.25, 0.25, -1), Direction(0, 0, 1));
    auto intersection = triangle.intersect(ray);
    assert(intersection.has_value());
    assert(abs(intersection->distance - 1.0f) < EPSILON);
    std::cout << "Triangle intersection test passed!" << std::endl;
}

void test_cone_intersection() {
    Cone cone(Point(0, 0, 0), Direction(0, 1, 0), 1, 2, Material());
    Ray ray(Point(0, 0, -3), Direction(0, 0, 1));
    auto intersection = cone.intersect(ray);
    assert(intersection.has_value());
    assert(abs(intersection->distance - 2.0f) < EPSILON);
    std::cout << "Cone intersection test passed!" << std::endl;
}

void test_cylinder_intersection() {
    Cylinder cylinder(Point(0, 0, 0), Direction(0, 1, 0), 1, 2, Material());
    Ray ray(Point(0, 0, -3), Direction(0, 0, 1));
    auto intersection = cylinder.intersect(ray);
    assert(intersection.has_value());
    assert(abs(intersection->distance - 2.0f) < EPSILON);
    std::cout << "Cylinder intersection test passed!" << std::endl;
}

void run_intersect_tests() {
    std::cout << "Running intersect tests...\n";
    test_sphere_intersection();
    test_plane_intersection();
    test_triangle_intersection();
    test_cone_intersection();
    test_cylinder_intersection();
}
