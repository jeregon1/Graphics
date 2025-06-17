#include <iostream>
#include <vector>
#include <memory>

#include "object3D.hpp"
#include "pinholeCamera.hpp"
#include "Image.hpp"
#include "scene.hpp"

using namespace std;

void run_cornell_box_test(const string& sceneFile, const string& renderConfigFile = "configs/default_config.yaml") {

    // Load from scenes folder
    string scenePath = "scenes/" + sceneFile;
    auto sceneOpt = Scene::fromYAML(scenePath);
    if (!sceneOpt) {
        cerr << "Error: Could not load scene from " << scenePath << endl;
        return;
    }
    Scene& scene = sceneOpt->first;
    
    PinholeCamera camera;
    if (sceneOpt->second) {
        camera = *sceneOpt->second;  // Dereference the optional
        cout << "Successfully loaded scene with camera from " << scenePath << endl;
    } else {
        camera = PinholeCamera(Point(0, 0, -3));  // Create default camera
        cout << "Scene loaded without camera, using default camera." << endl;
    }
    
    cout << "Camera: " << camera.toString() << endl;
    cout << "Scene: " << scene.toString() << endl;

    auto configOpt = RenderConfig::fromYAML(renderConfigFile);
    RenderConfig config;
    if (configOpt) {
        cout << "Using render config from " << renderConfigFile << endl;
        config = *configOpt;
    } else {
        cout << "Using default render config." << endl;
        config = RenderConfig();
    }

    cout << "Rendering scene..." << endl;
    Image image = camera.render(scene, config);  // Reduced samples for faster testing
    
    string newFile = sceneFile.substr(0, sceneFile.length()-5);
    image.writePPM(newFile + ".ppm");
    cout << "Rendered to "+ newFile +".ppm" << endl;
}

int main(int argc, char* argv[]) {
    cout << "Running Cornell Box test...\n";
    
    string sceneFile = "cornell_box.yaml";  // default
    if (argc > 1) {
        sceneFile = argv[1];
    }
    
    cout << "Using scene file: " << sceneFile << endl;
    run_cornell_box_test(sceneFile);
    cout << "Cornell Box test completed.\n";
    return 0;
}

