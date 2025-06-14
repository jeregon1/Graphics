#include <iostream>
#include <cassert>
#include <string>
#include <iomanip>
#include "../include/Image.hpp"
#include "../include/toneMapping.hpp"

#define ppmTestFile     "assets/mpi_office.ppm"
#define ppmTestFileCopy "assets/mpi_officeCopy.ppm"
const std::string OUTPUT_DIR = "test_outputs/";

void test_readWritePPM(const std::string& file) {
    auto optImg = Image::readPPM(file);
    if (!optImg) {
        std::cerr << "Error: Could not load PPM file " << file << std::endl;
        return;
    }
    Image img = std::move(*optImg);
    
    img.writePPM(OUTPUT_DIR + ppmTestFileCopy);

    // Check if the file was created
    std::ifstream fileStream(ppmTestFileCopy);
    assert(fileStream.good());
    fileStream.close();
    
    // Check if the file has the same content
    auto optImgCopy = Image::readPPM(ppmTestFileCopy);
    if (!optImgCopy) {
        std::cerr << "Error: Could not load copy of PPM file" << std::endl;
        return;
    }
    Image imgCopy = std::move(*optImgCopy);
    assert(img.width == imgCopy.width);
    assert(img.height == imgCopy.height);
    for (size_t i = 0; i < img.pixels.size(); i++) {
        if (img.pixels[i] != imgCopy.pixels[i]) {
            std::cout << std::setprecision(40);
            std::cout << "Pixel " << i << " is different" << std::endl;
            std::cout << "Original: " << img.pixels[i] << std::endl;
            std::cout << "Copy:     " << imgCopy.pixels[i] << std::endl;

            break;
        }
    }
}

void testClamp(const std::string& path) {
    auto optImage = Image::readPPM(path);
    if (!optImage) {
        std::cerr << "Error: Could not load PPM file " << path << std::endl;
        return;
    }
    Image image = std::move(*optImage);
    ToneMapping::clamp(image);
    image.writePPM(OUTPUT_DIR + "clamp.ppm");
}

void testEqualization(const std::string& path) {
    auto optImage = Image::readPPM(path);
    if (!optImage) {
        std::cerr << "Error: Could not load PPM file " << path << std::endl;
        return;
    }
    Image image = std::move(*optImage);
    ToneMapping::equalization(image);
    image.writePPM(OUTPUT_DIR + "equalized.ppm");
}

void testEqualizationClamp(const std::string& path) {
    auto optImage = Image::readPPM(path);
    if (!optImage) {
        std::cerr << "Error: Could not load PPM file " << path << std::endl;
        return;
    }
    Image image = std::move(*optImage);
    ToneMapping::equalizationClamp(image);
    image.writePPM(OUTPUT_DIR + "equalizedClamp.ppm");
}

void testGamma(const std::string& path) {
    auto optImage = Image::readPPM(path);
    if (!optImage) {
        std::cerr << "Error: Could not load PPM file " << path << std::endl;
        return;
    }
    Image image = std::move(*optImage);
    ToneMapping::gamma(image);
    image.writePPM(OUTPUT_DIR + "gamma.ppm");
}

void testClampGamma(const std::string& path) {
    auto optImage = Image::readPPM(path);
    if (!optImage) {
        std::cerr << "Error: Could not load PPM file " << path << std::endl;
        return;
    }
    Image image = std::move(*optImage);
    ToneMapping::clampGamma(image);
    image.writePPM(OUTPUT_DIR + "clampGamma.ppm");
}

void test_toneMapping(const std::string& path) {
    testClamp(path);
    testEqualization(path);
    testEqualizationClamp(path);
    testGamma(path);
    testClampGamma(path);
}

void test_readWriteBMP(const std::string& file) {

    auto optImg1 = Image::readPPM(file);
    if (!optImg1) {
        std::cerr << "Error: Could not load PPM file " << file << std::endl;
        return;
    }
    Image img1 = std::move(*optImg1);
    
    img1.writeBMP(OUTPUT_DIR + "test.bmp");

    // Check if the file was created
    std::ifstream fileStream("test.bmp");
    assert(fileStream.good());
    fileStream.close();

    auto optImg2 = Image::readBMP("test.bmp");
    if (!optImg2) {
        std::cerr << "Error: Could not load BMP file test.bmp" << std::endl;
        return;
    }
    Image img2 = std::move(*optImg2);
    
    // Check if the file has the same content
    assert(img1.width == img2.width);
    assert(img1.height == img2.height);
    
    std::cout << std::setprecision(10);
    for (size_t i = 0; i < img1.pixels.size(); i++) {
        if (img1.pixels[i] != img2.pixels[i]) {
            std::cout << "Pixel " << i << " is different" << std::endl;
            std::cout << "Original: " << img1.pixels[i] << std::endl;
            std::cout << "Copy:     " << img2.pixels[i] << std::endl;

            break;
        } else {
            std::cout << "Pixel " << i << " is the same" << std::endl;
            std::cout << "Original: " << img1.pixels[i] << std::endl;
            std::cout << "Copy:     " << img2.pixels[i] << std::endl;
            break;
        }
        std::cout << std::endl;
    }
}

void run_p2_tests(const std::string& file = ppmTestFile) {
    test_readWritePPM(file);
    test_toneMapping(file);
    test_readWriteBMP(file);
}
