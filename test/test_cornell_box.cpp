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
    scene.addObject(make_shared<Plane>(Direction(1, 0, 0), RGB(1, 0, 0))); // Right plane (Green)
    scene.addObject(make_shared<Plane>(Direction(-1, 0, 0), RGB(0, 1, 0))); // Left plane (Red)
    scene.addObject(make_shared<Plane>(Direction(0, 1, 0), RGB(0.5, 0.5, 0.5))); // Floor plane 
    scene.addObject(make_shared<Plane>(Direction(0, -1, 0), RGB(0.5, 0.5, 0.5))); // Ceiling plane 
    scene.addObject(make_shared<Plane>(Direction(0, 0, 1), RGB(0.5, 0.5, 0.5))); // Back plane 

    // Spheres-
    // Estas intersecciones funcionan pero se detecta rarote
    scene.addObject(make_shared<Sphere>(Point(-0.5, 0.7, 0.25), 0.3, RGB(1,1,1))); // Left sphere
    scene.addObject(make_shared<Sphere>(Point(0.5, 0.7, -0.25), 0.3, RGB(1,1,1))); // Right sphere

    // Lights
    shared_ptr<PointLight> shared_pointLight = make_shared<PointLight>(Point(0, 0.15, 0), RGB(1, 1, 1)); // Light source
    scene.addLight(shared_pointLight); // Light source

    // Camera
    PinholeCamera camera(Point(0, 0, -3.5), 1, 256, 256);

    // Render
    Image image = camera.renderPathTracing(scene, 8);
    image.writePPM("output.ppm");
    //image.writeBMP("output.bmp");

    return 0;
}

