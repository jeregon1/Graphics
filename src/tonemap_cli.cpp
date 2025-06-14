/*
 * tonemap_cli.cpp
 * Date: 2024/09/30
 * Author: CLI for testing tone mapping operators and image formats
 * Description: Command-line interface for tone mapping operations
 */

#include <iostream>
#include <string>
#include <vector>
#include "../include/Image.hpp"
#include "../include/toneMapping.hpp"

using namespace std;

void printUsage(const string& programName) {
    cout << "Usage: " << programName << " <input_file> <output_file> <operation> [parameters]\n"
         << "\nOperations:\n"
         << "  clamp [max_value=1.0]              - Clamp values to max_value\n"
         << "  equalize [max_value=auto]          - Linear equalization\n"
         << "  equalize-clamp [max_value=1.0]     - Equalization + clamp\n"
         << "  gamma [gamma_value=2.2]            - Gamma correction\n"
         << "  clamp-gamma [max=1.0] [gamma=2.2]  - Clamp + gamma correction\n"
         << "  reinhard [key=0.18] [white=1.0]    - Reinhard tone mapping\n"
         << "  convert                            - Just convert between formats\n"
         << "\nSupported formats: .ppm, .bmp\n"
         << "\nExamples:\n"
         << "  " << programName << " input.ppm output.bmp convert\n"
         << "  " << programName << " hdr.ppm ldr.ppm clamp 1.0\n"
         << "  " << programName << " input.ppm output.ppm gamma 2.2\n"
         << "  " << programName << " input.ppm output.ppm reinhard 0.18 1.5\n";
}

int main(int argc, char* argv[]) {
    if (argc < 4) {
        printUsage(argv[0]);
        return 1;
    }

    string inputFile = argv[1];
    string outputFile = argv[2];
    string operation = argv[3];

    try {
        // Load the image
        cout << "Loading image: " << inputFile << endl;
        Image image(inputFile);
        
        if (image.pixels.empty()) {
            cerr << "Error: Could not load image " << inputFile << endl;
            return 1;
        }
        
        cout << "Image loaded successfully: " << image.width << "x" << image.height << " pixels" << endl;
        cout << "Max value in image: " << image.max() << endl;

        // Apply the tone mapping operation
        if (operation == "clamp") {
            float maxValue = (argc > 4) ? stof(argv[4]) : 1.0f;
            cout << "Applying clamp with max value: " << maxValue << endl;
            ToneMapping::clamp(image, maxValue);
        }
        else if (operation == "equalize") {
            float maxValue = (argc > 4) ? stof(argv[4]) : 0.0f; // 0 means auto-detect
            cout << "Applying equalization" << (maxValue > 0 ? " with max value: " + to_string(maxValue) : " (auto-detect max)") << endl;
            ToneMapping::equalization(image, maxValue);
        }
        else if (operation == "equalize-clamp") {
            float maxValue = (argc > 4) ? stof(argv[4]) : 1.0f;
            cout << "Applying equalization + clamp with max value: " << maxValue << endl;
            ToneMapping::equalizationClamp(image, maxValue);
        }
        else if (operation == "gamma") {
            float gammaValue = (argc > 4) ? stof(argv[4]) : 2.2f;
            cout << "Applying gamma correction with gamma: " << gammaValue << endl;
            ToneMapping::gamma(image, gammaValue);
        }
        else if (operation == "clamp-gamma") {
            float maxValue = (argc > 4) ? stof(argv[4]) : 1.0f;
            float gammaValue = (argc > 5) ? stof(argv[5]) : 2.2f;
            cout << "Applying clamp + gamma with max: " << maxValue << ", gamma: " << gammaValue << endl;
            ToneMapping::clampGamma(image, maxValue, gammaValue);
        }
        else if (operation == "reinhard") {
            float key = (argc > 4) ? stof(argv[4]) : 0.18f;
            float white = (argc > 5) ? stof(argv[5]) : 1.0f;
            cout << "Applying Reinhard tone mapping with key: " << key << ", white: " << white << endl;
            ToneMapping::reinhard(image, key, white);
        }
        else if (operation == "convert") {
            cout << "Converting between formats (no tone mapping applied)" << endl;
        }
        else {
            cerr << "Error: Unknown operation '" << operation << "'" << endl;
            printUsage(argv[0]);
            return 1;
        }

        // Save the result
        cout << "Saving image: " << outputFile << endl;
        string extension = outputFile.substr(outputFile.find_last_of(".") + 1);
        
        if (extension == "ppm") {
            image.writePPM(outputFile);
        } else if (extension == "bmp") {
            image.writeBMP(outputFile);
        } else {
            cerr << "Error: Unsupported output format '" << extension << "'" << endl;
            return 1;
        }
        
        cout << "Operation completed successfully!" << endl;
        cout << "Final max value in image: " << image.max() << endl;

    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }

    return 0;
}
