#include <iostream>
#include <vector>
#include <memory>

#include "../include/object3D.hpp"
#include "../include/Image.hpp"

using namespace std;

int main() {

    Scene scene;

    // Planes
    // Estas intersecciones no funcionan, no se detecta nada

/*
    scene.addObject(make_shared<Plane>(Direction(1, 0, 0), 1, RGB(0.5, 0.1, 0.5))); // Left plane
    scene.addObject(make_shared<Plane>(Direction(-1, 0, 0), 1, RGB(1, 0, 0))); // Right plane
    scene.addObject(make_shared<Plane>(Direction(0, 1, 0), 1, RGB(1, 1, 0))); // Floor plane
    scene.addObject(make_shared<Plane>(Direction(0, -1, 0), 1, RGB(1, 0, 0))); // Ceiling plane
    scene.addObject(make_shared<Plane>(Direction(0, 0, -1), 1, RGB(1, 0, 1))); // Back plane
*/
    // Spheres
    // Estas intersecciones funcionan pero se detecta rarote
    scene.addObject(make_shared<Sphere>(Point(-0.5, -0.7, 0.25), 0.3)); // Left sphere
    scene.addObject(make_shared<Sphere>(Point(0.5, -0.7, -0.25), 0.3)); // Right sphere

    // Camera
    PinholeCamera camera(Point(0, 0, -1), Direction(-1, 0, 0), Direction(0, 1, 0), Direction(0, 0, 3), 10, 256, 256);

    // Render
    Image image(256, 256, camera.render(scene));
    image.writePPM("output.ppm");





    return 0;
}

