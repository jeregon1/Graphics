#pragma once

#include <iostream>
#include <cmath>
#include <sstream>
#include <array>
#include <stdexcept>

using namespace std;

using Matrix4x4 = array<array<float, 4>, 4>;

class Coordinate {
public:
    float x, y, z;

    Coordinate(float x, float y, float z);

    float& operator[](int index);

    virtual string toString() const;

    friend ostream& operator<<(ostream& os, const Coordinate& c);
};

class Direction : public Coordinate {
public:
    Direction(float x, float y, float z);

    Direction operator+(const Direction& other) const;
    Direction operator-(const Direction& other) const;
    Direction operator*(float scalar) const;
    Direction operator/(float scalar) const;
    float mod() const;
    Direction normalize() const;
    float dot(const Direction& other) const;
    Direction cross(const Direction& other) const;
};

class Point : public Coordinate {
public:
    Coordinate base;

    Point(const Coordinate& base, float x, float y, float z);

    float dot(const Point& other) const;

    string toString() const override;
};

class Transform {
private:
    static array<float, 4> multiplyMatrixByVector(const Matrix4x4& matrix, const float vector[4]);

public:
    static Coordinate translate(const Coordinate& axis, const Point& point);
    static Coordinate rotate_x(float theta, const Direction& direction);
    static Coordinate rotate_y(float theta, const Direction& direction);
    static Coordinate rotate_z(float theta, const Direction& direction);
    static Coordinate scale(float factor_x, float factor_y, float factor_z, const Coordinate& c);
};

class Sphere {
public:
    Point base;
    float inclinacion, azimut;

    Sphere(const Point& base, const float& inclinacion, const float& azimut);

    string toString() const;
};

class Ray {
public:
    Point origin;
    Direction direction;

    Ray(const Point& origin, const Direction& direction);

    // Returns the point at a distance t from the origin
    Point at(float t) const;
};

/*  
How to define a geometric primitive:
• Implicit equation f(x, y, z)

The surface of the geometry is defined by
all points (x, y, z) such that f(x, y, z) = 0
 */
class Plane {
public:
    Point base;
    Direction normal;

    Plane(const Point& base, const Direction& normal);

    // Returns the distance from the plane to a point
    float distance(const Point& point) const;
};