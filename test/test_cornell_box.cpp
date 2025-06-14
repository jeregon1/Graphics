#include <iostream>
#include <vector>
#include <memory>

#include "../include/object3D.hpp"
#include "../include/pinholeCamera.hpp"
#include "../include/Image.hpp"

using namespace std;

void run_cornell_box_test() {

    Scene scene;

    // Materials
    Material redMaterial(RGB(0.8, 0.2, 0.2), RGB(0, 0, 0)); 
    Material greenMaterial(RGB(0.2, 0.8, 0.2), RGB(0, 0, 0)); 
    Material blueMaterial(RGB(0.2, 0.2, 0.8), RGB(0, 0, 0)); 
    Material greyMaterial(RGB(0.5, 0.5, 0.5), RGB(0, 0, 0)); 
    Material plasticMaterial(RGB(0.9, 0.9, 0.9), RGB(0.1, 0.1, 0.1)); 
    Material glossyMaterial(RGB(0.8, 0.8, 0.8), RGB(0.5, 0.5, 0.5)); 
    Material yellowMaterial(RGB(0.8, 0.8, 0.2), RGB(0, 0, 0));
    Material purpleMaterial(RGB(0.8, 0.2, 0.8), RGB(0, 0, 0));

    // Cornell Box walls
    scene.addObject(make_shared<Plane>(Direction(1, 0, 0), redMaterial, 1)); // Right wall (red)
    scene.addObject(make_shared<Plane>(Direction(-1, 0, 0), greenMaterial, 1)); // Left wall (green)
    scene.addObject(make_shared<Plane>(Direction(0, 1, 0), greyMaterial, 1)); // Floor 
    scene.addObject(make_shared<Plane>(Direction(0, -1, 0), greyMaterial, 1)); // Ceiling
    scene.addObject(make_shared<Plane>(Direction(0, 0, 1), greyMaterial, 2)); // Back wall

    // Test all primitives positioned properly within the cornell box
    // Spheres
    scene.addObject(make_shared<Sphere>(Point(-0.4, -0.3, 0.3), 0.2, plasticMaterial)); 
    scene.addObject(make_shared<Sphere>(Point(0.4, -0.3, -0.3), 0.2, glossyMaterial)); 

    // Triangle
    scene.addObject(make_shared<Triangle>(
        Point(-0.3, 0.2, 0.5), Point(0.0, -0.1, 0.5), Point(0.3, 0.2, 0.5), 
        blueMaterial));

    // Cylinder - positioned on the floor
    scene.addObject(make_shared<Cylinder>(
        Point(-0.4, -1.0, -0.2), Direction(0, 1, 0), 0.1, 0.4, 
        yellowMaterial));

    // Cone - positioned on the floor
    scene.addObject(make_shared<Cone>(
        Point(0.3, -1.0, -0.4), Direction(0, 1, 0), 0.15, 0.5, 
        purpleMaterial));

    // Light positioned inside the box
    shared_ptr<PointLight> shared_pointLight = make_shared<PointLight>(Point(0, 0.5, 0), RGB(8, 8, 8)); 
    scene.addLight(shared_pointLight); 

    // Camera positioned closer to see all objects
    PinholeCamera camera(Point(0, 0, -0.5), 45, 512, 512);

    cout << "Rendering scene with all primitives..." << endl;
    Image image = camera.renderPathTracing(scene, 16);  // Reduced samples for faster testing
    image.writePPM("test_all_primitives.ppm");
    cout << "Rendered to test_all_primitives.ppm" << endl;
}

