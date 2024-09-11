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

using namespace std;

class Base {
public:
    double x, y, z;

    Base(double x = 0, double y = 0, double z = 0) : x(x), y(y), z(z) {}
};

class Direction {
public:
    double x, y, z;

    Direction(double x = 0, double y = 0, double z = 0) : x(x), y(y), z(z) {}

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
        double magnitude = mod();
        return *this / magnitude;
    }

    double dot(const Direction& other) const {
        return x * other.x + y * other.y + z * other.z;
    }

    Direction cross(const Direction& other) const {
        return Direction(y * other.z - z * other.y, z * other.x - x * other.z, x * other.y - y * other.x);
    }

    double operator[](int index) {
        assert (index >= 0 && index < 3);
        if (index == 0) return x;
        if (index == 1) return y;
        if (index == 2) return z;
    }

    string toString() const {
        ostringstream oss;
        oss << "(" << x << ", " << y << ", " << z << ")";
        return oss.str();
    }

    // Overload the << operator
    friend ostream& operator<<(ostream& os, const Point& point) {
        os << point.base.x << " + (" << point.direction.x << ", " << point.direction.y << ", " << point.direction.z << ")";
        return os;
    }
};

class Point {
public:
    Base base;
    Direction direction;

    Point(const Base& base, const Direction& direction) : base(base), direction(direction) {}

    double dot(const Point& other) const {
        return direction.dot(other.direction);
    }

    double operator[](int index) {
        return direction[index];
    }

    string toString() const {
        ostringstream oss;
        oss << base.x << " + (" << direction.x << ", " << direction.y << ", " << direction.z << ")";
        return oss.str();
    }

    // Overload the << operator
    friend ostream& operator<<(ostream& os, const Point& point) {
        os << point.base.x << " + (" << point.direction.x << ", " << point.direction.y << ", " << point.direction.z << ")";
        return os;
    }
};

#include <array>
class Transform  {

    using matrix4x4 = array<array<double, 4>, 4>;
    private:
        mamtrix4x4& multiply(const mamtrix4x4& matrix1, const double[4] vector) {
            
            double result[4][4] = {0};
            for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 4; j++) {
                    result[i][j] += matrix1[i][j] * vector[j];
                }
            }

            return result;
        }

    public:

        Transform() {}
        
        

        Point translate(const Direction& axis, const Point& point) {
            double matrix[4][4] = {
                {1, 0, 0, axis.x},
                {0, 1, 0, axis.y},
                {0, 0, 1, axis.z},
                {0, 0, 0, 1}
            };
            // Multiply the matrix by the point
            double result[4] = {0, 0, 0, 0};
            for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 4; j++) {
                    result[i] += matrix[i][j] * point.direction[j];
                }
            }

            return Point(Base(result[0], result[1], result[2]), point.direction);
        }

        Direction rotate_x(int theta, Direction direction) {
            const double matrix[4][4]  = {
                {1, 0, 0, 0},
                {0, cos(theta), -sin(theta), 0},
                {0, sin(theta), cos(theta), 0},
                {0, 0, 0, 1}
            };

            // Multiply the matrix by the point
            double result[4] = {0, 0, 0, 0};
            for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 4; j++) {
                    result[i] += matrix[i][j] * direction[j];
                }
            }

        }

        Direction rotate_y(int theta) {

        }

        Direction rotate_z(int theta) {

        }

        Direction rotate(int theta, Direction  direction) {
            Direction normalizedDirection = direction.normalize();
            Transform auxx = rotate_x(normalizedDirection.x * theta);
            Transform auxy = rotate_y(normalizedDirection.y * theta);
            Transform auxz = rotate_z(normalizedDirection.z * theta);
            return auxx.multiply(auxy.multiply(auxz));
        }


        Transform scale() {

        }
};

class Sphere {
public:
    Point base;
    Direction inclinacion;
    Direction azimut;

    Sphere(const Point& base, const Direction& inclinacion, const Direction& azimut)
        : base(base), inclinacion(inclinacion), azimut(azimut) {}

    string toString() const {
        ostringstream oss;
        oss << "Base: " << base.toString() << "\n"
            << "Inclinacion: " << inclinacion.toString() << "\n"
            << "Azimut: " << azimut.toString();
        return oss.str();
    }
};

int main() {
    // Example usage
    Base base(1, 2, 3);
    Direction dir1(4, 5, 6);
    Direction dir2(7, 8, 9);
    Point point(base, dir1);
    Sphere sphere(point, dir1, dir2);

    cout << "Point: " << point.toString() << endl;
    cout << "Matrix multiplication:\n" << (mat1 * mat2).toString() << endl;
    cout << "Sphere:\n" << sphere.toString() << endl;

    return 0;
}