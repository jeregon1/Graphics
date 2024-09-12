#ifndef MAIN_H
#define MAIN_H

#include <iostream>
#include <cmath>
#include <sstream>
#include <array>
#include <stdexcept>

using namespace std;

class Coordinate {
public:
    double x, y, z;

    Coordinate(double x = 0, double y = 0, double z = 0);

    double& operator[](int index);

    virtual string toString() const;

    friend ostream& operator<<(ostream& os, const Coordinate& c);
};

class Direction : public Coordinate {
public:
    Direction(double x = 0, double y = 0, double z = 0);

    Direction operator+(const Direction& other) const;
    Direction operator-(const Direction& other) const;
    Direction operator*(double scalar) const;
    Direction operator/(double scalar) const;
    double mod() const;
    Direction normalize() const;
    double dot(const Direction& other) const;
    Direction cross(const Direction& other) const;
};

class Point : public Coordinate {
public:
    Coordinate base;

    Point(const Coordinate& base, double x = 0, double y = 0, double z = 0);

    double dot(const Point& other) const;

    string toString() const override;
};

class Transform {
    using Matrix4x4 = array<array<double, 4>, 4>;

private:
    static array<double, 4> multiplyMatrixByVector(const Matrix4x4& matrix, const double vector[4]);

public:
    Transform();

    static Coordinate translate(const Coordinate& axis, const Point& point);
    static Coordinate rotate_x(int theta, const Direction& direction);
    static Coordinate rotate_y(int theta, const Direction& direction);
    static Coordinate rotate_z(int theta, const Direction& direction);
    static Coordinate scale(double factor_x, double factor_y, double factor_z, const Coordinate& c);
};

class Sphere {
public:
    Point base;
    double inclinacion, azimut;

    Sphere(const Point& base, const double& inclinacion, const double& azimut);

    string toString() const;
};

#endif // MAIN_H
