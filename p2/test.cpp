// p2/test_test.cpp

#include <iostream>
#include <cassert>
#include <string>
#include <iomanip>
#include "Image.hpp"
#include "toneMapping.hpp"

using namespace std;

string ppmTestFile =     "../HDR PPM files-20240910/mpi_office.ppm";
string ppmTestFileCopy = "../HDR PPM files-20240910/mpi_officeCopy.ppm";

void test_readWritePPM(const string& file) {
    Image img = Image::readPPM(file);
    
    img.writePPM(ppmTestFileCopy, 10000000, 5);

    // Check if the file was created
    ifstream fileStream(ppmTestFileCopy);
    assert(fileStream.good());
    fileStream.close();
    
    // Check if the file has the same content
    Image imgCopy = Image::readPPM(ppmTestFileCopy);
    assert(img.width == imgCopy.width);
    assert(img.height == imgCopy.height);
    for (size_t i = 0; i < img.pixels.size(); i++) {
        if (img.pixels[i] != imgCopy.pixels[i]) {
            cout << setprecision(40);
            cout << "Pixel " << i << " is different" << endl;
            cout << "Original: " << img.pixels[i] << endl;
            cout << "Copy:     " << imgCopy.pixels[i] << endl;

            if (img.pixels[i].r != imgCopy.pixels[i].r) {
                cout << "R is different" << endl;
            } else if (img.pixels[i].g != imgCopy.pixels[i].g) {
                cout << "G is different" << endl;
            } else if (img.pixels[i].b != imgCopy.pixels[i].b) {
                cout << "B is different" << endl;
            }
            break;
        }
    }
}

void testClamp(const string& path) {
    Image image = Image::readPPM(path);
    clamp(image);
    image.writePPM("clamp.ppm", 10000000, 5);
}

void testEqualization(const string& path) {
    Image image = Image::readPPM(path);
    equalization(image);
    image.writePPM("equalized.ppm", 10000000, 5);
}

void testEqualizationClamp(const string& path) {
    Image image = Image::readPPM(path);
    equalizationClamp(image);
    image.writePPM("equalizedClamp.ppm", 10000000, 5);
}

void testGamma(const string& path) {
    Image image = Image::readPPM(path);
    gamma(image);
    image.writePPM("gamma.ppm", 10000000, 5);
}

void testClampGamma(const string& path) {
    Image image = Image::readPPM(path);
    clampGamma(image);
    image.writePPM("clampGamma.ppm", 10000000, 5);
}

void test_toneMapping(const string& path) {
    testClamp(path);
    testEqualization(path);
    testEqualizationClamp(path);
    testGamma(path);
    testClampGamma(path);
}

int main() {
    test_readWritePPM(ppmTestFile);

    test_toneMapping(ppmTestFile);
    
    return 0;
}
