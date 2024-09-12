// p1/test_main.cpp

#include <iostream>
#include <cassert>

#include "geometry.hpp" // Adjust the path as necessary

void test_translate() {
    Direction axis(1, 2, 3);
    Point point(Coordinate(0, 0, 0), 4, 5, 6);
    Coordinate result = Transform::translate(axis, point);
    assert(result.x == 5);
    assert(result.y == 7);
    assert(result.z == 9);
    std::cout << "test_translate passed!" << std::endl;
}

void test_rotate_x() {
    Direction direction(1, 0, 0);
    Coordinate result = Transform::rotate_x(90, direction);
    assert(abs(result.x - 1) < 1e-9);
    assert(abs(result.y - 0) < 1e-9);
    assert(abs(result.z - 0) < 1e-9);
    std::cout << "test_rotate_x passed!" << std::endl;
}

void test_rotate_y() {
    Direction direction(0, 1, 0);
    Coordinate result = Transform::rotate_y(90, direction);
    assert(abs(result.x - 0) < 1e-9);
    assert(abs(result.y - 1) < 1e-9);
    assert(abs(result.z - 0) < 1e-9);
    std::cout << "test_rotate_y passed!" << std::endl;
}

void test_rotate_z() {
    Direction direction(0, 0, 1);
    Coordinate result = Transform::rotate_z(90, direction);
    assert(abs(result.x - 0) < 1e-9);
    assert(abs(result.y - 0) < 1e-9);
    assert(abs(result.z - 1) < 1e-9);
    std::cout << "test_rotate_z passed!" << std::endl;
}

void test_scale() {
    Coordinate c(1, 2, 3);
    Coordinate result = Transform::scale(2, 3, 4, c);
    assert(result.x == 2);
    assert(result.y == 6);
    assert(result.z == 12);
    std::cout << "test_scale passed!" << std::endl;
}

int main() {
    test_translate();
    test_rotate_x();
    test_rotate_y();
    test_rotate_z();
    test_scale();
    std::cout << "All tests passed!" << std::endl;
    return 0;
}