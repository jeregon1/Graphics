#pragma once

#include <iostream>
#include <cmath>
#include <sstream>
#include <array>

using namespace std;

using Matrix4x4 = array<array<float, 4>, 4>;

class Coordinate {
public:
    float x, y, z;

    Coordinate(float x = 0, float y = 0, float z = 0);

    float& operator[](int index);

    virtual string toString() const;

    friend ostream& operator<<(ostream& os, const Coordinate& c);
};


class Direction;

class Point : public Coordinate {
public:
    Point(float x = 0, float y = 0, float z = 0);

    float dot(const Point& other) const;

    Direction operator-(const Point& p2) const;
    Point operator*(float scalar) const;
    Point operator+(const Direction& other) const;
    string toString() const override;
};

class Direction : public Coordinate {
public:
    Direction(float x = 0, float y = 0, float z = 0);

    Direction operator+(const Direction& other) const;
    Direction operator-(const Direction& other) const;
    double operator*(const Direction& other) const; // Dot product
    Direction operator*(float scalar) const;
    Direction operator/(float scalar) const;
    bool operator==(const Direction& other) const;
    Point operator+(const Point& point) const;
    float mod() const;
    Direction normalize() const; // Returns a unit vector
    float dot(const Direction& other) const;
    Direction cross(const Direction& other) const;
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
