/*
 * test_bmp.cpp
 * Simple test program to validate BMP reading/writing functionality
 */

#include <iostream>
#include <cassert>
#include "../include/Image.hpp"
#include "../include/toneMapping.hpp"

using namespace std;

#define OUTPUT_DIR "test_outputs/"

int main() {
    cout << "=== BMP Format Test ===" << endl;
    
    try {
        // Test 1: Load PPM and save as BMP
        cout << "\n1. Loading PPM file..." << endl;
        auto optOriginalImage = Image::readPPM("assets/mpi_office.ppm");
        
        if (!optOriginalImage) {
            cerr << "Error: Could not load PPM file" << endl;
            return 1;
        }
        
        Image originalImage = std::move(*optOriginalImage);
        
        cout << "   Loaded: " << originalImage.width << "x" << originalImage.height << " pixels" << endl;
        cout << "   Max value: " << originalImage.max() << endl;
        
        // Test 2: Save as BMP
        cout << "\n2. Saving as BMP..." << endl;
        originalImage.writeBMP(OUTPUT_DIR + "test_output.bmp");
        
        // Test 3: Load the BMP back
        cout << "\n3. Loading BMP file back..." << endl;
        auto optLoadedImage = Image::readBMP("test_output.bmp");
        
        if (!optLoadedImage) {
            cerr << "Error: Could not load BMP file" << endl;
            return 1;
        }
        
        Image loadedImage = std::move(*optLoadedImage);
        
        cout << "   Loaded: " << loadedImage.width << "x" << loadedImage.height << " pixels" << endl;
        cout << "   Max value: " << loadedImage.max() << endl;
        
        // Test 4: Compare dimensions
        cout << "\n4. Comparing dimensions..." << endl;
        assert(originalImage.width == loadedImage.width);
        assert(originalImage.height == loadedImage.height);
        cout << "   ✓ Dimensions match" << endl;
        
        // Test 5: Compare pixel data (with tolerance for format conversion)
        cout << "\n5. Comparing pixel data..." << endl;
        const float tolerance = 1.0f / 255.0f; // Allow 1 unit difference due to 8-bit quantization
        int differentPixels = 0;
        float maxDifference = 0.0f;
        
        for (size_t i = 0; i < originalImage.pixels.size(); i++) {
            RGB orig = originalImage.pixels[i];
            RGB loaded = loadedImage.pixels[i];
            
            float diffR = std::abs(orig.r - loaded.r);
            float diffG = std::abs(orig.g - loaded.g);
            float diffB = std::abs(orig.b - loaded.b);
            float maxChannelDiff = std::max({diffR, diffG, diffB});
            
            if (maxChannelDiff > tolerance) {
                differentPixels++;
                maxDifference = std::max(maxDifference, maxChannelDiff);
            }
        }
        
        cout << "   Different pixels: " << differentPixels << " / " << originalImage.pixels.size() << endl;
        cout << "   Max difference: " << maxDifference << endl;
        
        // Test 6: Apply tone mapping and save as BMP
        cout << "\n6. Testing tone mapping with BMP output..." << endl;
        Image gammaImage = originalImage; // Copy
        ToneMapping::gamma(gammaImage, 2.2f);
        gammaImage.writeBMP(OUTPUT_DIR + "test_gamma.bmp");
        cout << "   ✓ Gamma correction applied and saved as BMP" << endl;
        
        // Test 7: Test format conversion chain
        cout << "\n7. Testing format conversion chain..." << endl;
        gammaImage.writePPM(OUTPUT_DIR + "test_gamma.ppm");
        auto optChainTest = Image::readPPM("test_gamma.ppm");
        if (!optChainTest) {
            cerr << "Error: Could not load intermediate PPM file" << endl;
            return 1;
        }
        Image chainTest = std::move(*optChainTest);
        chainTest.writeBMP(OUTPUT_DIR + "test_chain.bmp");
        cout << "   ✓ PPM -> BMP -> PPM -> BMP conversion chain completed" << endl;
        
        cout << "\n=== All BMP tests passed! ===" << endl;
        
    } catch (const std::exception& e) {
        cerr << "Test failed with exception: " << e.what() << endl;
        return 1;
    }
    
    return 0;
}
