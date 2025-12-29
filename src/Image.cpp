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

std::vector<float> make_gaussian_kernel(float sigma) noexcept {

    // Se toma un radio de 3*sigma porque fuera de ese rango
    // la gaussiana es prácticamente 0
    const int radius = static_cast<int>(std::ceil(3.0f * sigma));

    // Tamaño total del kernel: [-radius, ..., 0, ..., +radius]
    const int size = 2 * radius + 1;

    std::vector<float> kernel(size);
    float sum = 0.0f; // Para normalizar después

    // Se recorre el kernel usando índices centrados en 0
    for (int i = -radius; i <= radius; ++i) {

        // Fórmula de la gaussiana 1D (sin el factor constante,
        // porque se normaliza después)
        float v = std::exp(-(i * i) / (2.0f * sigma * sigma));

        // i va de [-radius, radius], así que se desplaza para indexar el vector
        kernel[i + radius] = v;

        // Se acumula la suma total de los pesos
        sum += v;
    }

    // Normalización: asegura que la suma del kernel sea 1
    // (no cambia el brillo global de la imagen)
    for (float& v : kernel)
        v /= sum;

    return kernel;
}


// Aplica el filtro gaussiano SOLO en horizontal
Image gaussian_blur_horizontal(const Image& src,
                               const std::vector<float>& k) noexcept {

    // Imagen temporal donde se guarda el resultado
    Image tmp(src.width, src.height);

    // Radio del kernel (mitad del tamaño)
    const int r = static_cast<int>(k.size() / 2);

    for (int y = 0; y < src.height; ++y) {
        for (int x = 0; x < src.width; ++x) {

            // Acumulador del color resultante
            RGB acc{0, 0, 0};

            // Convolución horizontal: solo se desplaza en X
            for (int i = -r; i <= r; ++i) {

                // Se maneja el borde clampeando al píxel más cercano
                int xx = std::clamp(x + i, 0, src.width - 1);

                // Píxel vecino
                const RGB& p = src.at(xx, y);

                // Peso correspondiente del kernel
                float w = k[i + r];

                // Se acumula cada canal de color
                acc.r += p.r * w;
                acc.g += p.g * w;
                acc.b += p.b * w;
            }

            // Se guarda el píxel filtrado
            tmp.pixels[y * tmp.width + x] = acc;
        }
    }

    return tmp;
}


// Aplica el filtro gaussiano SOLO en vertical
Image gaussian_blur_vertical(const Image& src,
                             const std::vector<float>& k) noexcept {

    Image dst(src.width, src.height);
    const int r = static_cast<int>(k.size() / 2);

    for (int y = 0; y < src.height; ++y) {
        for (int x = 0; x < src.width; ++x) {

            RGB acc{0, 0, 0};

            // Convolución vertical: solo se desplaza en Y
            for (int i = -r; i <= r; ++i) {

                // Manejo de bordes en vertical
                int yy = std::clamp(y + i, 0, src.height - 1);

                const RGB& p = src.at(x, yy);
                float w = k[i + r];

                acc.r += p.r * w;
                acc.g += p.g * w;
                acc.b += p.b * w;
            }

            dst.pixels[y * dst.width + x] = acc;
        }
    }

    return dst;
}

// Función pública: aplica el blur gaussiano completo (2D)
Image Image::gaussianBlur(const Image& src, float sigma) noexcept {

    auto kernel = make_gaussian_kernel(sigma);
    Image tmp = gaussian_blur_horizontal(src, kernel);
    return gaussian_blur_vertical(tmp, kernel);

}

// Aplica un filtro bilateral a la imagen de entrada
// sigma_space controla el tamaño espacial del filtro
// sigma_color controla cuánto se preservan los bordes
Image Image::bilateralFilter(const Image& src,
                             float sigma_space,
                             float sigma_color) noexcept
{
    Image dst(src.width, src.height);

    // Radio espacial: igual que en la gaussiana, se corta en 3*sigma
    const int radius =
        static_cast<int>(std::ceil(3.0f * sigma_space));

    // Precalculamos constantes para evitar divisiones en el bucle
    const float inv_2_sigma_space2 = 1.0f / (2.0f * sigma_space * sigma_space);
    const float inv_2_sigma_color2 = 1.0f / (2.0f * sigma_color * sigma_color);

    for (int y = 0; y < src.height; ++y) {
        for (int x = 0; x < src.width; ++x) {

            // Color del píxel central
            const RGB& center = src.at(x, y);

            // Acumulador del color resultante
            RGB acc{0, 0, 0};

            // Acumulador de pesos (para normalizar)
            float weight_sum = 0.0f;

            // Ventana 2D alrededor del píxel
            for (int j = -radius; j <= radius; ++j) {
                for (int i = -radius; i <= radius; ++i) {

                    // Coordenadas vecinas con manejo de bordes
                    int xx = std::clamp(x + i, 0, src.width  - 1);
                    int yy = std::clamp(y + j, 0, src.height - 1);

                    const RGB& p = src.at(xx, yy);

                    // Distancia espacial al píxel central (x, y)
                    float dist_space = static_cast<float>(i * i + j * j);

                    // Distancia de color (euclídea al cuadrado)
                    float dr = p.r - center.r;
                    float dg = p.g - center.g;
                    float db = p.b - center.b;
                    float dist_color = dr * dr + dg * dg + db * db;

                    // Peso espacial (gaussiana espacial)
                    float w_space = std::exp(-dist_space * inv_2_sigma_space2);

                    // Peso de rango (gaussiana de color)
                    float w_color = std::exp(-dist_color * inv_2_sigma_color2);

                    // Peso bilateral total
                    float w = w_space * w_color;

                    // Acumulación ponderada del color
                    acc.r += p.r * w;
                    acc.g += p.g * w;
                    acc.b += p.b * w;

                    weight_sum += w;
                }
            }

            // Normalización final
            acc.r /= weight_sum;
            acc.g /= weight_sum;
            acc.b /= weight_sum;

            dst.at(x, y) = acc;
        }
    }

    return dst;
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
    // for (int i = 0; i < height*width; i++) {
    // }
    for (int i = height - 1; i >= 0; i--) { // Write from top row (height-1) to bottom row (0)
        for (int j = i* width; j < (i+1)*width; j++) {
            file >> pixels[j];
            pixels[j] *= maxColorRatio;
        }
    }

    file.close();
    return Image(width, height, std::move(pixels));
}

bool Image::writePPM(const std::string& path, int colorResolution) const noexcept {
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
    file << colorResolution << "\n";  // Disk color resolution
    
    file << std::fixed << std::setprecision(0); // Sin decimales
    for (int i = height - 1; i >= 0; i--) { // Write from top row (height-1) to bottom row (0)
        int j = i * width;
        if (imageMax > 1.0f) {
            // HDR image: normalize by actual max value, then scale to colorResolution
            file << round((pixels[j] / imageMax) * colorResolution); // First element
            for (j = j + 1; j < (i + 1) * width; j++) {
                file << "     " << round((pixels[j] / imageMax) * colorResolution);
            }
        } else {
            // LDR image: values are already in [0,1], just scale to colorResolution
            file << round(pixels[j].clamp() * colorResolution); // First element  
            for (j = j + 1; j < (i + 1) * width; j++) {
                file << "     " << round(pixels[j].clamp() * colorResolution);
            }
        }
        file << "\n";
    }
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
