#include "../include/object3D.hpp"
#include "../include/Image.hpp"
#include <vector>
#include <fstream>
#include <random>

using namespace std;

Image PinholeCamera::render(const Scene& scene, unsigned samplesPerPixel) const {
    vector<RGB> pixels(height * width);

    for (int y = 0; y < height; y++) {
        float normalizedY = static_cast<float>(y) - (height / 2);
        for (int x = 0; x < width; x++) {
            float normalizedX = static_cast<float>(x) - (width / 2);
            // Calculate the color of the pixel at (x, y)
            RGB pixelColor = calculatePixelColor(scene, normalizedX, normalizedY, samplesPerPixel);
            pixels[y * width + x] = pixelColor;
        }
    }

    return Image(width, height, pixels);
}

RGB PinholeCamera::calculatePixelColor(const Scene& scene, float x, float y, unsigned samplesPerPixel) const {
    static random_device rd;
    static mt19937 gen(rd());
    static uniform_real_distribution<float> dis(0.0, 1.0);

    RGB accumulatedColor(0, 0, 0);

    for (unsigned i = 0; i < samplesPerPixel; i++) { // Example for image (100x100), x = 0, y = 0, samplesPerPixel = 1
        // Generate a random offset for anti-aliasing
        float x_offset = x + dis(gen); // -50.5
        float y_offset = y + dis(gen); // -50.5

        // Generate a ray through the pixel
        Ray ray = generateRay(x_offset, y_offset);

        // Trace the ray and accumulate the color
        accumulatedColor += traceRay(ray, scene);
    }

    // Average the accumulated color
    return accumulatedColor / samplesPerPixel;
}

Ray PinholeCamera::generateRay(float x, float y) const {
    // Calculate the direction of the ray
    Direction direction = (left * x + up * y - forward);
    return Ray(origin, direction.normalize());
}

RGB PinholeCamera::traceRay(const Ray& ray, const Scene& scene) const {
    // Find the closest intersection of the ray with the scene
    auto intersection = scene.intersect(ray);

    // Return the color of the intersected material, or black if no intersection
    if (intersection.has_value()) {
        return intersection->material;
    }
    return RGB(0, 0, 0);
}
