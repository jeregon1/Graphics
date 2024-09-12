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

#include <iostream>
#include <cmath>
#include <sstream>
#include <cassert>
#include <array>

using namespace std;

class Coordinate {
public:
    double x, y, z;

    Coordinate(double x = 0, double y = 0, double z = 0) : x(x), y(y), z(z) {}
    
    double& operator[](int index) {
        switch (index) {
            case 0: return x;
            case 1: return y;
            case 2: return z;
            default: throw out_of_range("Index out of range accessing Coordinate");
        }
    }

    virtual string toString() const {
        ostringstream oss;
        oss << "(" << x << ", " << y << ", " << z << ")";
        return oss.str();
    }

    friend ostream& operator<<(ostream& os, const Coordinate& c) {
        return os << c.toString();
    }
};

class Direction : public Coordinate {
public:

    Direction(double x = 0, double y = 0, double z = 0) : Coordinate{x, y, z} {}

    Direction operator+(const Direction& other) const {
        return Direction(x + other.x, y + other.y, z + other.z);
    }

    Direction operator-(const Direction& other) const {
        return Direction(x - other.x, y - other.y, z - other.z);
    }

    Direction operator*(double scalar) const {
        return Direction(x * scalar, y * scalar, z * scalar);
    }

    Direction operator/(double scalar) const {
        return Direction(x / scalar, y / scalar, z / scalar);
    }

    double mod() const {
        return sqrt(x * x + y * y + z * z);
    }

    Direction normalize() const {
        return *this / mod();
    }

    double dot(const Direction& other) const {
        return x * other.x + y * other.y + z * other.z;
    }

    Direction cross(const Direction& other) const {
        return Direction(y * other.z - z * other.y, 
                         z * other.x - x * other.z, 
                         x * other.y - y * other.x);
    }

};

class Point : public Coordinate {
public:
    Coordinate base;

    Point(const Coordinate& base, double x = 0, double y = 0, double z = 0) : Coordinate{x, y, z}, base(base) {}

    double dot(const Point& other) const {
        return (x - base.x) * (other.x - other.base.x) + 
               (y - base.y) * (other.y - other.base.y) + 
               (z - base.z) * (other.z - other.base.z);
    }

     string toString() const override {
        ostringstream oss;
        oss << base.x << " + " << Coordinate::toString();
        return oss.str();
    }

};

class Transform {
    using Matrix4x4 = array<array<double, 4>, 4>;

private:
    static array<double, 4> multiplyMatrixByVector(const Matrix4x4& matrix, const double vector[4]) {
        array<double, 4> result = {0, 0, 0, 0};
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                result[i] += matrix[i][j] * vector[j];
            }
        }
        return result;
    }

public:
    Transform() {}

    static Coordinate translate(const Coordinate& axis, const Point& point) {
        Matrix4x4 matrix = {{
            {1, 0, 0, axis.x},
            {0, 1, 0, axis.y},
            {0, 0, 1, axis.z},
            {0, 0, 0, 1}
        }};

        double vector[4] = {point.x, point.y, point.z, 1};
        array<double, 4> result = multiplyMatrixByVector(matrix, vector);

        return Point(result[0], result[1], result[2]);
    }

    static Coordinate rotate_x(int theta, const Direction& direction) {
        Matrix4x4 matrix = {{
            {1, 0, 0, 0},
            {0, cos(theta), -sin(theta), 0},
            {0, sin(theta), cos(theta), 0},
            {0, 0, 0, 1}
        }};

        double vector[4] = {direction.x, direction.y, direction.z, 1};
        array<double, 4> result = multiplyMatrixByVector(matrix, vector);

        return Direction(result[0], result[1], result[2]);
    }

    static Coordinate rotate_y(int theta, const Direction& direction) {
        Matrix4x4 matrix = {{
            {cos(theta), 0, sin(theta), 0},
            {0, 1, 0, 0},
            {-sin(theta), 0, cos(theta), 0},
            {0, 0, 0, 1}
        }};

        double vector[4] = {direction.x, direction.y, direction.z, 1};
        array<double, 4> result = multiplyMatrixByVector(matrix, vector);

        return Direction(result[0], result[1], result[2]);
    }

    static Coordinate rotate_z(int theta, const Direction& direction) {
        Matrix4x4 matrix = {{
            {cos(theta), -sin(theta), 0, 0},
            {sin(theta), cos(theta), 0, 0},
            {0, 0, 1, 0},
            {0, 0, 0, 1}
        }};

        double vector[4] = {direction.x, direction.y, direction.z, 1};
        array<double, 4> result = multiplyMatrixByVector(matrix, vector);

        return Direction(result[0], result[1], result[2]);
    }

    static Coordinate scale(double factor_x, double factor_y, double factor_z, const Coordinate& c) {

        Matrix4x4 matrix = {{
            {factor_x, 0, 0, 0},
            {0, factor_y, 0, 0},
            {0, 0, factor_z, 0},
            {0, 0, 0, 1}
        }};

        double vector[4] = {c.x, c.y, c.z, 1};
        array<double, 4> result = multiplyMatrixByVector(matrix, vector);

        return Coordinate(result[0], result[1], result[2]);
        }
};

class Sphere {
public:
    Point base;
    double inclinacion, azimut;

    Sphere(const Point& base, const double& inclinacion, const double& azimut)
        : base(base), inclinacion(inclinacion), azimut(azimut) {}

    string toString() const {
        ostringstream oss;
        oss << "Base: " << base.toString() << "\n"
            << "Inclinacion: " << inclinacion << "\n"
            << "Azimut: " << azimut;
        return oss.str();
    }
};
