/*
 * Image.cpp
 * Date: 2024/09/30
 * Author: Jesús López Ansón, 839922
 * Author: Pablo terés Pueyo, 843350
 * Description: Implementation of Image methods
 */

#include "../include/Image.hpp"
#include <iomanip>

using namespace std;

Image::Image(const string& filename){
    // Get the file extension
    string extension = filename.substr(filename.find_last_of(".") + 1);
    if (extension == "ppm") {
        *this = readPPM(filename);
    } else if (extension == "bmp") {
        *this = readBMP(filename);
    } else {
        cerr << "Invalid file extension in file " << filename << ". Found " << extension << " instead of ppm or bmp" << endl;
    }
    //*this = readPPM(filename);
}

float Image::max() const {
    float max = 0;
    for (const RGB &pixel : pixels) {
        max = std::max(max, pixel.max());
    }
    return max;
}

void skipComments(ifstream &file, float &diskColorRes) {
    while (file.peek() == '#') {
        string comment;
        getline(file, comment);
        if (comment.find("#MAX=") == 0) {
            diskColorRes = stof(comment.substr(5));
        }
    }
}

Image Image::readPPM(const string& path) {
    ifstream file(path);
    if (!file.is_open()) {
        cerr << "Error opening path" << endl;
        return Image();
    }

    float memoryColorResolution;

    skipComments(file, memoryColorResolution);

    string format;
    getline(file, format);
    if (!format.empty() && format.back() == '\r')
        format.pop_back();
    
    if (format != "P3") {
        cerr << "Invalid PPM format in file " << path << ". Found " << format << " instead of P3" << endl;
        return Image();
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

    vector<RGB> pixels(width * height);
    float maxColorRatio = memoryColorResolution / diskColorResolution;
    for (int i = 0; i < width * height; i++) {
        file >> pixels[i];
        pixels[i] *= maxColorRatio;
    }

    file.close();
    return Image(width, height, pixels, memoryColorResolution);
}

void Image::writePPM(const string& path, float diskColorRes) const {
    // If the path is not direct, get the filename for the comment in the file
    string filename = path;
    size_t found = path.find_last_of("/\\");
    if (found != string::npos) {
        filename = path.substr(found + 1);
    }
    
    ofstream file(path);
    if (!file.is_open()) {
        cerr << "Error opening path " + path << endl;
        return;
    }
    file << "P3\n";
    file << "# " << filename << "\n";
    file << width << " " << height << "\n";
    file << "#MAX=" << memoryColorResolution << "\n";
    file << (int) diskColorRes << "\n";
    cout << fixed << setprecision(0); // Sin decimales
    file << fixed << setprecision(0); // Sin decimales
    float maxColorRatio = diskColorRes / memoryColorResolution;
    for (int i = 0; i < height; i++) {
        int j = i * width;
        file << round(pixels[j] * maxColorRatio); // First element without left padding
        for (j = j + 1; j < (i + 1) * width; j++) {
            file << "     " << round(pixels[j] * maxColorRatio);
        }
        file << "\n";
    }

    file.close();
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

Image Image::readBMP(const string& path) {
    ifstream file(path, ios::binary);
    if (!file.is_open()) {
        cerr << "Error opening file " << path << endl;
        return Image();
    }

    BMPHeader header;
    BMPInfoHeader infoHeader;

    file.read(reinterpret_cast<char*>(&header), sizeof(header));
    file.read(reinterpret_cast<char*>(&infoHeader), sizeof(infoHeader));

    if (header.fileType != 0x4D42) {
        cerr << "Error: Not a BMP file" << endl;
        return Image();
    }

    Image image(infoHeader.width, abs(infoHeader.height));
    image.pixels.resize(image.width * image.height);

    file.seekg(header.offsetData, ios::beg);

    const int padding = (4 - (image.width * 3) % 4) % 4;
    for (int y = 0; y < image.height; ++y) {
        for (int x = 0; x < image.width; ++x) {
            uint8_t b, g, r;
            file.read(reinterpret_cast<char*>(&b), sizeof(b));
            file.read(reinterpret_cast<char*>(&g), sizeof(g));
            file.read(reinterpret_cast<char*>(&r), sizeof(r));
            image.pixels[y * image.width + x] = RGB(r, g, b) / 255.0f * Image::MEMORY_COLOR_RESOLUTION;
        }
        file.ignore(padding);
    }

    file.close();
    return image;
}

void Image::writeBMP(const string& path) const {
    ofstream file(path, ios::binary);
    if (!file.is_open()) {
        cerr << "Error opening file " << path << endl;
        return;
    }

    BMPHeader header;
    BMPInfoHeader infoHeader;
    unsigned bitsPerColor = 16;

    header.fileType = 0x4D42;
    header.fileSize = sizeof(header) + sizeof(infoHeader) + width * height * 6; // 6 bytes per pixel
    header.reserved1 = 0;
    header.reserved2 = 0;
    header.offsetData = sizeof(header) + sizeof(infoHeader);

    infoHeader.size = sizeof(infoHeader);
    infoHeader.width = width;
    infoHeader.height = height;
    infoHeader.planes = 1;
    infoHeader.bitCount = bitsPerColor * 3; // 48 bits per pixel
    infoHeader.compression = 0;
    infoHeader.sizeImage = 0;
    infoHeader.xPixelsPerMeter = 0;
    infoHeader.yPixelsPerMeter = 0;
    infoHeader.colorsUsed = 0;
    infoHeader.colorsImportant = 0;

    file.write(reinterpret_cast<const char*>(&header), sizeof(header));
    file.write(reinterpret_cast<const char*>(&infoHeader), sizeof(infoHeader));

    const int padding = (4 - (width * (infoHeader.bitCount / 8)) % 4) % 4;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            RGB pixel_normalized = pixels[y * width + x] / this->memoryColorResolution;
            uint16_t r = static_cast<uint16_t>(pixel_normalized.r * 65535);
            uint16_t g = static_cast<uint16_t>(pixel_normalized.g * 65535);
            uint16_t b = static_cast<uint16_t>(pixel_normalized.b * 65535);
            file.write(reinterpret_cast<const char*>(&r), sizeof(r));
            file.write(reinterpret_cast<const char*>(&g), sizeof(g));
            file.write(reinterpret_cast<const char*>(&b), sizeof(b));
        }
        for (int i = 0; i < padding; ++i) {
            uint8_t zero = 0;
            file.write(reinterpret_cast<const char*>(&zero), sizeof(zero));
        }
    }

    file.close();
}
