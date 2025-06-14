/*
 * Image.cpp
 * Date: 2024/09/30
 * Author: Jesús López Ansón, 839922
 * Author: Pablo terés Pueyo, 843350
 * Description: Implementation of Image methods
 */

#include "../include/Image.hpp"
#include <iomanip>
#include <iostream>
#include <fstream>
#include <cmath>
#include <cstdint>
#include <algorithm>

Image::Image(const std::string& filename){
    // Get the file extension
    std::string extension = filename.substr(filename.find_last_of(".") + 1);
    if (extension == "ppm") {
        if (auto img = readPPM(filename)) {
            *this = std::move(*img);
        }
    } else if (extension == "bmp") {
        if (auto img = readBMP(filename)) {
            *this = std::move(*img);
        }
    } else {
        std::cerr << "Invalid file extension in file " << filename << ". Found " << extension << " instead of ppm or bmp" << std::endl;
    }
}

float Image::max() const noexcept {
    float max = 0;
    for (const RGB &pixel : pixels) {
        max = std::max(max, pixel.max());
    }
    return max;
}

void skipComments(std::ifstream &file, float &diskColorRes) {
    while (file.peek() == '#') {
        std::string comment;
        std::getline(file, comment);
        if (comment.find("#MAX=") == 0) {
            diskColorRes = std::stof(comment.substr(5));
        }
    }
}

std::optional<Image> Image::readPPM(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Error opening path" << std::endl;
        return std::nullopt;
    }

    float memoryColorResolution = 255.0f; // Default value if no #MAX= comment found

    skipComments(file, memoryColorResolution);

    std::string format;
    std::getline(file, format);
    if (!format.empty() && format.back() == '\r')
        format.pop_back();
    
    if (format != "P3") {
        std::cerr << "Invalid PPM format in file " << path << ". Found " << format << " instead of P3" << std::endl;
        return std::nullopt;
    }

    skipComments(file, memoryColorResolution);
    int width, height;
    file >> width >> height;
    file.ignore(256, '\n');

    skipComments(file, memoryColorResolution);
    float diskColorResolution;
    file >> diskColorResolution;
    file.ignore(256, '\n');

    skipComments(file, memoryColorResolution);

    std::vector<RGB> pixels(width * height);
    float maxColorRatio = memoryColorResolution / diskColorResolution;
    for (int i = 0; i < width * height; i++) {
        file >> pixels[i];
        pixels[i] *= maxColorRatio;
    }

    file.close();
    return Image(width, height, std::move(pixels));
}

bool Image::writePPM(const std::string& path) const noexcept {
    // If the path is not direct, get the filename for the comment in the file
    std::string filename = path;
    size_t found = path.find_last_of("/\\");
    if (found != std::string::npos) {
        filename = path.substr(found + 1);
    }
    
    std::ofstream file(path);
    if (!file.is_open()) {
        std::cerr << "Error opening path " + path << std::endl;
        return false;
    }
    file << "P3\n";
    file << "# " << filename << "\n";
    
    // Calculate the actual max value in the image
    float imageMax = this->max();
    
    // Write HDR comment if the image has values > 1.0
    if (imageMax > 1.0f) {
        file << "#MAX=" << imageMax << "\n";
    }
    
    file << width << " " << height << "\n";
    file << "255" << "\n";  // Disk color resolution
    
    file << std::fixed << std::setprecision(0); // Sin decimales
    for (int i = 0; i < height; i++) {
        int j = i * width;
        if (imageMax > 1.0f) {
            // HDR image: normalize by actual max value, then scale to 255
            file << round((pixels[j] / imageMax) * 255); // First element
            for (j = j + 1; j < (i + 1) * width; j++) {
                file << "     " << round((pixels[j] / imageMax) * 255);
            }
        } else {
            // LDR image: values are already in [0,1], just scale to 255
            file << round(pixels[j].clamp() * 255); // First element  
            for (j = j + 1; j < (i + 1) * width; j++) {
                file << "     " << round(pixels[j].clamp() * 255);
            }
        }
        file << "\n";
    }
    std::cout << "Image written to " << path << std::endl;
    file.close();
    return true;
}

#pragma pack(push, 1)
struct BMPHeader {
    uint16_t fileType;        // File type, always 4D42h ("BM")
    uint32_t fileSize;        // Size of the file in bytes
    uint16_t reserved1;       // Always 0
    uint16_t reserved2;       // Always 0
    uint32_t offsetData;      // Start position of pixel data (bytes from the beginning of the file)
};

struct BMPInfoHeader {
    uint32_t size;            // Size of this header (in bytes)
    int32_t width;            // width of bitmap in pixels
    int32_t height;           // height of bitmap in pixels
    uint16_t planes;          // No. of planes for the target device, this is always 1
    uint16_t bitCount;        // No. of bits per pixel
    uint32_t compression;     // 0 or 3 - uncompressed
    uint32_t sizeImage;       // 0 - for uncompressed images
    int32_t xPixelsPerMeter;
    int32_t yPixelsPerMeter;
    uint32_t colorsUsed;      // No. color indexes in the color table. Use 0 for the max number of colors allowed by bit_count
    uint32_t colorsImportant; // No. of colors used for displaying the bitmap. If 0 all colors are required
};
#pragma pack(pop)

std::optional<Image> Image::readBMP(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error opening file " << path << std::endl;
        return std::nullopt;
    }

    BMPHeader header;
    BMPInfoHeader infoHeader;

    file.read(reinterpret_cast<char*>(&header), sizeof(header));
    file.read(reinterpret_cast<char*>(&infoHeader), sizeof(infoHeader));

    if (header.fileType != 0x4D42) {
        std::cerr << "Error: Not a BMP file" << std::endl;
        return std::nullopt;
    }

    if (infoHeader.bitCount != 24) {
        std::cerr << "Error: Only 24-bit BMP files are supported" << std::endl;
        return std::nullopt;
    }

    Image image(infoHeader.width, std::abs(infoHeader.height));
    image.pixels.resize(image.width * image.height);

    file.seekg(header.offsetData, std::ios::beg);

    const int padding = (4 - (image.width * 3) % 4) % 4;
    
    // BMP files store pixels bottom-to-top, so we need to reverse the row order
    for (int y = 0; y < image.height; ++y) {
        int targetRow = image.height - 1 - y; // Reverse row order
        for (int x = 0; x < image.width; ++x) {
            uint8_t b, g, r;
            file.read(reinterpret_cast<char*>(&b), sizeof(b));
            file.read(reinterpret_cast<char*>(&g), sizeof(g));
            file.read(reinterpret_cast<char*>(&r), sizeof(r));
            image.pixels[targetRow * image.width + x] = RGB(r, g, b) / 255.0f;
        }
        file.ignore(padding);
    }

    file.close();
    return image;
}

bool Image::writeBMP(const std::string& path) const noexcept {
    std::ofstream file(path, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error opening file " << path << std::endl;
        return false;
    }

    BMPHeader header;
    BMPInfoHeader infoHeader;

    const int padding = (4 - (width * 3) % 4) % 4;
    const int rowSize = width * 3 + padding;
    const int imageSize = rowSize * height;

    header.fileType = 0x4D42;
    header.fileSize = sizeof(header) + sizeof(infoHeader) + imageSize;
    header.reserved1 = 0;
    header.reserved2 = 0;
    header.offsetData = sizeof(header) + sizeof(infoHeader);

    infoHeader.size = sizeof(infoHeader);
    infoHeader.width = width;
    infoHeader.height = height;
    infoHeader.planes = 1;
    infoHeader.bitCount = 24; 
    infoHeader.compression = 0;
    infoHeader.sizeImage = imageSize;
    infoHeader.xPixelsPerMeter = 0;
    infoHeader.yPixelsPerMeter = 0;
    infoHeader.colorsUsed = 0;
    infoHeader.colorsImportant = 0;

    file.write(reinterpret_cast<const char*>(&header), sizeof(header));
    file.write(reinterpret_cast<const char*>(&infoHeader), sizeof(infoHeader));

    // Calculate the actual max value in the image for HDR normalization
    float imageMax = this->max();
    
    // Write pixel data (bottom-to-top row order, BGR format)
    for (int y = height - 1; y >= 0; --y) {
        for (int x = 0; x < width; ++x) {
            RGB pixel = pixels[y * width + x];
            
            // Handle HDR images by normalizing to [0,1] range before converting to 8-bit
            if (imageMax > 1.0f) {
                // HDR image: normalize by actual max value, then scale to 255
                pixel = pixel / imageMax;
            }
            
            // Clamp to [0,1] range and convert to 8-bit values
            pixel = pixel.clamp();
            uint8_t b = static_cast<uint8_t>(pixel.b * 255.0f);
            uint8_t g = static_cast<uint8_t>(pixel.g * 255.0f);
            uint8_t r = static_cast<uint8_t>(pixel.r * 255.0f);
            
            // Write in BGR order (BMP standard)
            file.write(reinterpret_cast<const char*>(&b), sizeof(b));
            file.write(reinterpret_cast<const char*>(&g), sizeof(g));
            file.write(reinterpret_cast<const char*>(&r), sizeof(r));
        }
        // Add row padding
        for (int i = 0; i < padding; ++i) {
            uint8_t zero = 0;
            file.write(reinterpret_cast<const char*>(&zero), sizeof(zero));
        }
    }

    file.close();
    return true;
}
