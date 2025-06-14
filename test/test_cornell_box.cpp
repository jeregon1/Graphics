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
    Material redMaterial(RGB(1, 0, 0), RGB(0, 0, 0)); // Red material
    Material greenMaterial(RGB(0, 1, 0), RGB(0, 0, 0)); // Green material
    Material greyMaterial(RGB(0.5, 0.5, 0.5), RGB(0, 0, 0)); // Grey material
    Material plasticMaterial(RGB(1, 1, 1), RGB(0.1, 0.1, 0.1)); // Plastic material
    Material glossyMaterial(RGB(1, 1, 1), RGB(0.5, 0.5, 0.5)); // Glossy material

    scene.addObject(make_shared<Plane>(Direction(1, 0, 0), redMaterial)); // Right plane (Green)
    scene.addObject(make_shared<Plane>(Direction(-1, 0, 0), greenMaterial)); // Left plane (Red)
    scene.addObject(make_shared<Plane>(Direction(0, 1, 0), greyMaterial)); // Floor plane 
    scene.addObject(make_shared<Plane>(Direction(0, -1, 0), greyMaterial)); // Ceiling plane 
    scene.addObject(make_shared<Plane>(Direction(0, 0, 1), greyMaterial)); // Back plane 

    // Spheres-
    // Estas intersecciones funcionan pero se detecta rarote
    scene.addObject(make_shared<Sphere>(Point(-0.5, 0.7, 0.25), 0.3, plasticMaterial)); // Left sphere
    scene.addObject(make_shared<Sphere>(Point(0.5, 0.7, -0.25), 0.3, glossyMaterial)); // Right sphere

    // Lights
    shared_ptr<PointLight> shared_pointLight = make_shared<PointLight>(Point(0, 0.15, 0), RGB(1, 1, 1)); // Light source
    scene.addLight(shared_pointLight); // Light source

    // Camera
    PinholeCamera camera(Point(0, 0, -3.5), 1, 256, 256);

    // Render
    Image image = camera.renderPathTracing(scene, 64);
    image.writePPM("output.ppm");
    //image.writeBMP("output.bmp");

    return 0;
}

