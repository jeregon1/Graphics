/* 
In the programming language of your choice, implement:
• Data basics: 3D points and directions
• Operations:
    • Addition, subtraction, scalar multiplication and scalar division
    • Modulus, normalization
    • Dot and cross products, pretty stdout operator
• Test your implementation with several examples
• Do you have extra time?
    • Matrices
    • Homogeneous coordinates. 3x3 matrices or 4x4?
    • Translation, rotation, change of scale, inverse transform. Combinations.
 */

#include "../include/geometry.hpp"

#include <iostream>
#include <cmath>
#include <sstream>
#include <cassert>
#include <array>

using namespace std;

/***************
 * Coordinates *
 ***************/

string Coordinate::toString() const {
    ostringstream oss;
    oss << "(" << x << ", " << y << ", " << z << ")";
    return oss.str();
}

/*********
 * Point *
 *********/
// Defined here to avoid circular dependency with Direction
Direction Point::operator-(const Point& p2) const {
    return Direction(x - p2.x, y - p2.y, z - p2.z);
}

Point Point::operator+(const Direction& other) const {
    return Point(x + other.x, y + other.y, z + other.z);
}

/*************
 * Direction *
 *************/

Direction Direction::normalize() const {
    float magnitude = mod();
    if (magnitude < 1e-9f) {
        // Cannot normalize a zero vector - throw exception or return zero vector
        throw invalid_argument("Cannot normalize a zero vector");
        // Alternative: return Direction(0, 0, 0); // Return zero vector
    }
    return Direction(x / magnitude, y / magnitude, z / magnitude);
}

/*************
 * Transform *
 *************/

array<float, 4> Transform::multiplyMatrixByVector(const Matrix4x4& matrix, const float vector[4]) {
    array<float, 4> result = {0, 0, 0, 0};
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            result[i] += matrix[i][j] * vector[j];
        }
    }
    return result;
}

Coordinate Transform::translate(const Coordinate& axis, const Point& point) {
    Matrix4x4 matrix = {{
        {1, 0, 0, axis.x},
        {0, 1, 0, axis.y},
        {0, 0, 1, axis.z},
        {0, 0, 0, 1}
    }};

    float vector[4] = {point.x, point.y, point.z, 1};
    array<float, 4> result = Transform::multiplyMatrixByVector(matrix, vector);

    return Coordinate(result[0], result[1], result[2]);
}

Coordinate Transform::rotate_x(float theta, const Direction& direction) {
    Matrix4x4 matrix = {{
        {1, 0, 0, 0},
        {0, cos(theta), -sin(theta), 0},
        {0, sin(theta), cos(theta), 0},
        {0, 0, 0, 1}
    }};

    float vector[4] = {direction.x, direction.y, direction.z, 1};
    array<float, 4> result = multiplyMatrixByVector(matrix, vector);

    return Direction(result[0], result[1], result[2]);
}

Coordinate Transform::rotate_y(float theta, const Direction& direction) {
    Matrix4x4 matrix = {{
        {cos(theta), 0, sin(theta), 0},
        {0, 1, 0, 0},
        {-sin(theta), 0, cos(theta), 0},
        {0, 0, 0, 1}
    }};

    float vector[4] = {direction.x, direction.y, direction.z, 1};
    array<float, 4> result = multiplyMatrixByVector(matrix, vector);

    return Direction(result[0], result[1], result[2]);
}

Coordinate Transform::rotate_z(float theta, const Direction& direction) {
    Matrix4x4 matrix = {{
        {cos(theta), -sin(theta), 0, 0},
        {sin(theta), cos(theta), 0, 0},
        {0, 0, 1, 0},
        {0, 0, 0, 1}
    }};

    float vector[4] = {direction.x, direction.y, direction.z, 1};
    array<float, 4> result = multiplyMatrixByVector(matrix, vector);

    return Direction(result[0], result[1], result[2]);
}

Coordinate Transform::scale(float factor_x, float factor_y, float factor_z, const Coordinate& c) {

    Matrix4x4 matrix = {{
        {factor_x, 0, 0, 0},
        {0, factor_y, 0, 0},
        {0, 0, factor_z, 0},
        {0, 0, 0, 1}
    }};

    float vector[4] = {c.x, c.y, c.z, 1};
    array<float, 4> result = multiplyMatrixByVector(matrix, vector);

    return Coordinate(result[0], result[1], result[2]);
}
