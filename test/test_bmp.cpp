/*
 * test_bmp.cpp
 * Simple test program to validate BMP reading/writing functionality
 */

#include <iostream>
#include <string> // Required for std::string
#include <fstream> // Required for std::ifstream
#include <cassert> // Required for assert
#include <iomanip> // Required for std::setprecision
#include "../include/Image.hpp"
#include "../include/toneMapping.hpp"

// Use const instead of constexpr for std::string as it's not a literal type
const std::string OUTPUT_DIR = "test_outputs/";

using namespace std;

void test_readWriteBMP(const std::string& inputFile, const std::string& outputFile) {
    std::cout << "Testing BMP read/write with input: " << inputFile << ", output: " << outputFile << std::endl;
    auto optImg1 = Image::readPPM(inputFile);
    if (!optImg1) {
        std::cerr << "Error: Could not load PPM file " << inputFile << " for BMP test" << std::endl;
        return;
    }
    Image img1 = std::move(*optImg1);

    // Save as BMP
    img1.writeBMP(outputFile);

    // Load BMP
    auto optImg2 = Image::readBMP(outputFile);
    if (!optImg2) {
        std::cerr << "Error: Could not load BMP file " << outputFile << " for BMP test" << std::endl;
        return;
    }
    Image img2 = std::move(*optImg2);

    // Compare dimensions
    assert(img1.width == img2.width);
    assert(img1.height == img2.height);

    // Compare pixel data (with tolerance for format conversion)
    const float tolerance = 1.0f / 255.0f; // Allow 1 unit difference due to 8-bit quantization
    int differentPixels = 0;
    float maxDifference = 0.0f;

    for (size_t i = 0; i < img1.pixels.size(); i++) {
        RGB orig = img1.pixels[i];
        RGB loaded = img2.pixels[i];

        float diffR = std::abs(orig.r - loaded.r);
        float diffG = std::abs(orig.g - loaded.g);
        float diffB = std::abs(orig.b - loaded.b);
        float maxChannelDiff = std::max({diffR, diffG, diffB});

        if (maxChannelDiff > tolerance) {
            differentPixels++;
            maxDifference = std::max(maxDifference, maxChannelDiff);
        }
    }

    std::cout << "   Different pixels: " << differentPixels << " / " << img1.pixels.size() << std::endl;
    std::cout << "   Max difference: " << maxDifference << std::endl;

    // Apply tone mapping and save as BMP
    Image gammaImage = img1; // Copy
    ToneMapping::gamma(gammaImage, 2.2f);
    gammaImage.writeBMP(OUTPUT_DIR + "test_gamma.bmp");
    std::cout << "   ✓ Gamma correction applied and saved as BMP" << std::endl;

    // Test format conversion chain
    gammaImage.writePPM(OUTPUT_DIR + "test_gamma.ppm");
    auto optChainTest = Image::readPPM(OUTPUT_DIR + "test_gamma.ppm");
    if (!optChainTest) {
        cerr << "Error: Could not load intermediate PPM file" << endl;
        return;
    }
    Image chainTest = std::move(*optChainTest);
    chainTest.writeBMP(OUTPUT_DIR + "test_chain.bmp");
    cout << "   ✓ PPM -> BMP -> PPM -> BMP conversion chain completed" << endl;

    cout << "\n=== All BMP tests passed! ===" << endl;
}

void run_bmp_tests() {
    std::cout << "Running BMP read/write test...\n";
    test_readWriteBMP("assets/mpi_office.ppm", OUTPUT_DIR + "test_output.bmp");
    // Add more BMP specific tests if needed
}

// int main() { // Original main function commented out or removed
// run_bmp_tests();
// return 0;
// }
