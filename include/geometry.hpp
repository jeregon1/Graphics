#pragma once

#include <iostream>
#include <cmath>
#include <sstream>
#include <array>


using Matrix4x4 = std::array<std::array<float, 4>, 4>;

class Coordinate {
public:
    float x, y, z;

    Coordinate(float x = 0, float y = 0, float z = 0) : x(x), y(y), z(z) {}

    const float& operator[](int index) const {
        switch (index) {
            case 0: return x;
            case 1: return y;
            case 2: return z;
            default: throw std::out_of_range("Index out of range accessing Coordinate");
        }
    }

    virtual std::string toString() const;
    friend std::ostream& operator<<(std::ostream& os, const Coordinate& c) {
        return os << c.toString();
    }
};


class Direction;

class Point : public Coordinate {
public:
    Point(float x = 0, float y = 0, float z = 0) : Coordinate{x, y, z} {}

    float dot(const Point& other) const {
        return x * other.x + y * other.y + z * other.z;
    }
    
    Direction operator-(const Point& p2) const;

    Point operator*(float scalar) const {
        return Point{x * scalar, y * scalar, z * scalar};
    }
    
    Point operator+(const Direction& other) const;
    
    std::string toString() const override {
        std::ostringstream oss;
        oss << "Point" << Coordinate::toString();
        return oss.str();
    }
};

class Direction : public Coordinate {
public:
    Direction(float x = 1, float y = 0, float z = 0) : Coordinate{x, y, z} {}

    Direction operator+(const Direction& other) const {
        return Direction(x + other.x, y + other.y, z + other.z);
    }
    
    Direction operator-(const Direction& other) const {
        return Direction(x - other.x, y - other.y, z - other.z);
    }
    
    Direction operator-() const { return Direction(-x, -y, -z); } // Negation
    
    double operator*(const Direction& other) const { // Dot product
        return x * other.x + y * other.y + z * other.z;
    }
    
    Direction operator*(float scalar) const {
        return Direction(x * scalar, y * scalar, z * scalar);
    }
    
    Direction operator/(float scalar) const {
        return Direction(x / scalar, y / scalar, z / scalar);
    }
    
    bool operator==(const Direction& other) const {
        return (x == other.x && y == other.y && z == other.z);
    }
    
    Point operator+(const Point& point) const {
        return Point(x + point.x, y + point.y, z + point.z);
    }
    
    float mod() const { return sqrt(x * x + y * y + z * z); }
    
    float dot(const Direction& other) const {
        return x * other.x + y * other.y + z * other.z;
    }
    
    Direction cross(const Direction& other) const {
        return Direction(y * other.z - z * other.y, 
                        z * other.x - x * other.z, 
                        x * other.y - y * other.x);
    }
    
    Direction normalize() const; // Returns a unit vector (kept in .cpp due to error handling)
};

class Transform {
private:
    static std::array<float, 4> multiplyMatrixByVector(const Matrix4x4& matrix, const float vector[4]);

public:
    static Coordinate translate(const Coordinate& axis, const Point& point);
    static Coordinate rotate_x(float theta, const Direction& direction);
    static Coordinate rotate_y(float theta, const Direction& direction);
    static Coordinate rotate_z(float theta, const Direction& direction);
    static Coordinate scale(float factor_x, float factor_y, float factor_z, const Coordinate& c);
};
