
#include <iostream>
#include <cassert>
#include <string>
#include <iomanip>
#include "../include/Image.hpp"
#include "../include/toneMapping.hpp"

using namespace std;

#define ppmTestFile     "/mnt/c/Users/jesus/OneDrive/Documentos/4ºCarrera/Gráfica/Prácticas/assets/mpi_office.ppm"
#define ppmTestFileCopy "/mnt/c/Users/jesus/OneDrive/Documentos/4ºCarrera/Gráfica/Prácticas/assets/mpi_officeCopy.ppm"

void test_readWritePPM(const string& file) {
    Image img = Image::readPPM(file);
    
    img.writePPM(ppmTestFileCopy);

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

            break;
        }
    }
}

void testClamp(const string& path) {
    Image image = Image::readPPM(path);
    clamp(image);
    image.writePPM("clamp.ppm");
}

void testEqualization(const string& path) {
    Image image = Image::readPPM(path);
    equalization(image);
    image.writePPM("equalized.ppm");
}

void testEqualizationClamp(const string& path) {
    Image image = Image::readPPM(path);
    equalizationClamp(image);
    image.writePPM("equalizedClamp.ppm");
}

void testGamma(const string& path) {
    Image image = Image::readPPM(path);
    gamma(image);
    image.writePPM("gamma.ppm");
}

void testClampGamma(const string& path) {
    Image image = Image::readPPM(path);
    clampGamma(image);
    image.writePPM("clampGamma.ppm");
}

void test_toneMapping(const string& path) {
    testClamp(path);
    testEqualization(path);
    testEqualizationClamp(path);
    testGamma(path);
    testClampGamma(path);
}

void test_readWriteBMP(const string& file) {

    Image img1 = Image::readPPM(file);
    
    img1.writeBMP("test.bmp");

    // Check if the file was created
    ifstream fileStream("test.bmp");
    assert(fileStream.good());
    fileStream.close();

    Image img2 = Image::readBMP("test.bmp");
    
    // Check if the file has the same content
    assert(img1.width == img2.width);
    assert(img1.height == img2.height);
    
    cout << setprecision(10);
    for (size_t i = 0; i < img1.pixels.size(); i++) {
        if (img1.pixels[i] != img2.pixels[i]) {
            cout << "Pixel " << i << " is different" << endl;
            cout << "Original: " << img1.pixels[i] << endl;
            cout << "Copy:     " << img2.pixels[i] << endl;

            break;
        } else {
            cout << "Pixel " << i << " is the same" << endl;
            cout << "Original: " << img1.pixels[i] << endl;
            cout << "Copy:     " << img2.pixels[i] << endl;
            break;
        }
        cout << endl;
    }
}

int main() {
    // test_readWritePPM(ppmTestFile);

    // test_toneMapping(ppmTestFile);

    test_readWriteBMP(ppmTestFile);
    
    return 0;
}
