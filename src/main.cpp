#include <iostream>
#include <vector>
#include <memory>
#include <string>
#include <cstring>

#include "object3D.hpp"
#include "pinholeCamera.hpp"
#include "Image.hpp"
#include "scene.hpp"
#include "toneMapping.hpp"

using namespace std;

// Default file names
const string DEFAULT_SCENE_FILE = "cornell_box.yaml";
const string DEFAULT_CONFIG_FILE = "configs/default_pathtracing_config.yaml";

void printHelp(const char* programName) {
    cout << "Graphics Renderer - Cornell Box and Scene Tester\n\n";
    cout << "USAGE:\n";
    cout << "  " << programName << " [OPTIONS] [SCENE] [CONFIG]\n";
    cout << "  " << programName << " [SCENE] [CONFIG]\n\n";
    
    cout << "ARGUMENTS:\n";
    cout << "  SCENE     Scene file (default: " << DEFAULT_SCENE_FILE << ", from scenes/ folder)\n";
    cout << "  CONFIG    Config file (default: " << DEFAULT_CONFIG_FILE << ")\n\n";
    
    cout << "OPTIONS:\n";
    cout << "  -h, --help        Show this help message\n";
    cout << "  -s SCENE          Specify scene file explicitly (from scenes/ folder)\n";
    cout << "  -c CONFIG         Specify config file explicitly\n";
    cout << "  -o OUTPUT         Specify output filename (default: scene name without extension)\n\n";
    
    cout << "EXAMPLES:\n";
    cout << "  " << programName << "                            # Use defaults\n";
    cout << "  " << programName << " simple_test.yaml           # Custom scene, default config\n";
    cout << "  " << programName << " scene.yaml config.yaml     # Custom scene and config\n";
    cout << "  " << programName << " -c custom_config.yaml      # Default scene, custom config\n";
    cout << "  " << programName << " -s scene.yaml -o final     # Explicit scene and output name\n";

    cout << "NOTES:\n";
    cout << "  - Scene files are loaded from 'scenes/' directory but config files can be loaded from anywhere\n";
    cout << "  - Output filename defaults to scene name + .ppm extension\n";
}

void renderImage(const string& sceneFile, const string& renderConfigFile, const string& outputFile) {

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

    // Store the desired tone mapping for later (apply once at the end)
    ToneMappingType desiredToneMapping = config.toneMapping;
    // Disable tone mapping in camera.render() to avoid double tone mapping
    config.toneMapping = ToneMappingType::NONE;

    cout << "Rendering scene..." << endl;
    
    Image image;
    
    // Check if bilateral filter is enabled
    if (config.useBilateralFilter) {
        cout << "Bilateral filter enabled (sigma_space=" << config.bilateralSigmaSpace 
             << ", sigma_color=" << config.bilateralSigmaColor << ")" << endl;
        cout << "Rendering direct and indirect lighting separately..." << endl;
        
        // Create config for direct lighting only
        RenderConfig directConfig = config;
        directConfig.lightingDecomposition = LightingDecomposition::DIRECT_ONLY;
        directConfig.toneMapping = ToneMappingType::NONE;  // Disable tone mapping here
        // Use fewer samples for direct lighting; most noise comes from indirect
        directConfig.samplesPerPixel = std::max(1, directConfig.samplesPerPixel / 4);
        
        // Create config for indirect lighting only
        RenderConfig indirectConfig = config;
        indirectConfig.lightingDecomposition = LightingDecomposition::INDIRECT_ONLY;
        indirectConfig.toneMapping = ToneMappingType::NONE;  // Disable tone mapping here
        
        // Render direct lighting
        cout << "  Rendering direct lighting..." << endl;
        Image directImage = camera.render(scene, directConfig);
        
        // Render indirect lighting
        cout << "  Rendering indirect lighting..." << endl;
        Image indirectImage = camera.render(scene, indirectConfig);
        
        // Apply bilateral filter to indirect lighting only
        cout << "  Applying bilateral filter to indirect lighting..." << endl;
        Image dummy;  // Temporary instance for calling the filter
        Image indirectFiltered = dummy.bilateralFilter(indirectImage, 
                                                        config.bilateralSigmaSpace, 
                                                        config.bilateralSigmaColor);
        
        // Combine direct + filtered indirect
        cout << "  Combining direct + filtered indirect..." << endl;
        image = Image(directImage.width, directImage.height);
        for (size_t i = 0; i < directImage.pixels.size(); ++i) {
            image.pixels[i] = directImage.pixels[i] + indirectFiltered.pixels[i];
        }
        
    } else {
        // Standard rendering without bilateral filter
        image = camera.render(scene, config);
    }

    // Apply tone mapping to the combined image (matching test_bilateral.cpp approach)
    if (desiredToneMapping == ToneMappingType::GAMMA) {
        ToneMapping::gamma(image, config.toneMappingGamma);
    } else if (desiredToneMapping == ToneMappingType::CLAMP) {
        ToneMapping::clamp(image, config.toneMappingMax);
    } else if (desiredToneMapping == ToneMappingType::EQUALIZATION) {
        ToneMapping::equalization(image);
    } else if (desiredToneMapping == ToneMappingType::EQUALIZATION_GAMMA) {
        ToneMapping::equalizationGamma(image, config.toneMappingGamma);
    } else if (desiredToneMapping == ToneMappingType::EQUALIZATION_CLAMP) {
        ToneMapping::equalizationClamp(image, config.toneMappingMax);
    } else if (desiredToneMapping == ToneMappingType::CLAMP_GAMMA) {
        ToneMapping::clampGamma(image, config.toneMappingMax, config.toneMappingGamma);
    } else if (desiredToneMapping == ToneMappingType::REINHARD) {
        ToneMapping::reinhard(image, config.toneMappingKey, config.toneMappingLwhite);
    }
    
    // Use provided output filename or derive from scene filename
    string finalOutputFile = outputFile;
    if (finalOutputFile.empty()) {
        string baseName = sceneFile.substr(0, sceneFile.length()-5);
        finalOutputFile = baseName + ".ppm";
    } else if (finalOutputFile.substr(finalOutputFile.length()-4) != ".ppm") {
        finalOutputFile += ".ppm";
    }
    
    image.writePPM(finalOutputFile);
    cout << "Rendered to " + finalOutputFile << endl;
}

int main(int argc, char* argv[]) {
    string sceneFile = DEFAULT_SCENE_FILE;
    string configFile = DEFAULT_CONFIG_FILE;
    string outputFile = "";  // Empty means derive from scene filename
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        string arg = argv[i];
        
        if (arg == "-h" || arg == "--help") {
            printHelp(argv[0]);
            return 0;
        }
        else if (arg == "-s") {
            if (i + 1 < argc) {
                sceneFile = argv[++i];
            } else {
                cerr << "Error: -s requires a scene file argument\n";
                return 1;
            }
        }
        else if (arg == "-c") {
            if (i + 1 < argc) {
                configFile = argv[++i];
            } else {
                cerr << "Error: -c requires a config file argument\n";
                return 1;
            }
        }
        else if (arg == "-o") {
            if (i + 1 < argc) {
                outputFile = argv[++i];
            } else {
                cerr << "Error: -o requires an output filename argument\n";
                return 1;
            }
        }
        else if (arg[0] != '-') {
            // Positional arguments
            if (i == 1) {
                sceneFile = arg;
            } else if (i == 2) {
                configFile = arg;
            } else {
                cerr << "Error: Too many positional arguments\n";
                printHelp(argv[0]);
                return 1;
            }
        }
        else {
            cerr << "Error: Unknown option '" << arg << "'\n";
            printHelp(argv[0]);
            return 1;
        }
    }
    
    cout << "Graphics Renderer - Starting...\n";
    cout << "Scene file: " << sceneFile << "\n";
    cout << "Config file: " << configFile << "\n";
    if (!outputFile.empty()) {
        cout << "Output file: " << outputFile << "\n";
    }
    cout << "\n";
    
    renderImage(sceneFile, configFile, outputFile);
    
    cout << "Rendering completed successfully.\n";
    return 0;
}

