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

    
    scene.addObject(make_shared<Plane>(Direction(1, 0, 0), 1, RGB(0, 1, 0))); // Left plane (Green)
    scene.addObject(make_shared<Plane>(Direction(-1, 0, 0), 1, RGB(1, 0, 0))); // Right plane (Red)
    scene.addObject(make_shared<Plane>(Direction(0, 1, 0), 1, RGB(1, 1, 0))); // Floor plane (Yellow)
    scene.addObject(make_shared<Plane>(Direction(0, -1, 0), 1, RGB(0, 1, 1))); // Ceiling plane (Cyan)
    scene.addObject(make_shared<Plane>(Direction(0, 0, -1), 1, RGB(1, 0, 1))); // Back plane (Magenta)

    // Spheres
    // Estas intersecciones funcionan pero se detecta rarote
    scene.addObject(make_shared<Sphere>(Point(-0.5, 0.7, 0.25), 0.3)); // Left sphere
    scene.addObject(make_shared<Sphere>(Point(0.5, 0.7, -0.25), 0.3)); // Right sphere
    //scene.addObject(make_shared<Sphere>(Point(0, 0, -2), 0.5)); // Top sphere

    // Camera

    PinholeCamera camera(Point(0, 0, 3.5), 1.5, 256, 256);

    // Render
    Image image = camera.render(scene, 8);
    image.writePPM("output.ppm");
    //image.writeBMP("output.bmp");


    return 0;
}

