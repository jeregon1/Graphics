
""" 
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
 """

from __future__ import annotations
import math

class Base:

    def __init__(self, x = 0, y = 0, z = 0):
        self.x = x
        self.y = y
        self.z = z

class Direction:

    def __init__(self, x = 0, y = 0, z = 0):
        self.x = x
        self.y = y
        self.z = z

    def __add__(self, other):
        return Direction(self.x + other.x, self.y + other.y, self.z + other.z)

    def __sub__(self, other):
        return Direction(self.x - other.x, self.y - other.y, self.z - other.z)

    def __mul__(self, scalar):
        return Direction(self.x * scalar, self.y * scalar, self.z * scalar)

    def __truediv__(self, scalar):
        return Direction(self.x / scalar, self.y / scalar, self.z / scalar)

    def mod(self):
        return math.sqrt(self.x ** 2 + self.y ** 2 + self.z ** 2)

    def normalize(self):
        return self / self.mod()

    def dot(self, other):
        return self.x * other.x + self.y * other.y + self.z * other.z

    def cross(self, other: Direction):
        return Direction(self.y * other.z - self.z * other.y, self.z * other.x - self.x * other.z, self.x * other.y - self.y * other.x)

    def __str__(self):  
        return f"({self.x}, {self.y}, {self.z})"


class Point:

    def __init__(self, base, direction: Direction):
        self.base = base
        self.direction = direction

    def dot(self, other):
        return self.direction.dot(other.direction)

    def __str__(self):
        return f"{self.base} + ({self.direction.x}, {self.direction.y}, {self.direction.z})"

class Matrix:
    
        def __init__(self, matrix):
            self.matrix = matrix
    
        def __mul__(self, other):
            return Matrix([[sum(a * b for a, b in zip(row, col)) for col in zip(*other.matrix)] for row in self.matrix])
    
        def __str__(self):
            return "\n".join(" ".join(str(cell) for cell in row) for row in self.matrix)

    
class Sphere:

    def __init__(self, base : Point, inclinacion : Direction, azimut : Direction):
        self.base = base
        self.inclinacion = inclinacion
        self.azimut = azimut
    
    def __str__(self):
        
