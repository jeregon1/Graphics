#include "../include/object3D.hpp"
#include "../include/Image.hpp"
#include <vector>
#include <fstream>

using namespace std;

Image PinholeCamera::render(const Scene& scene) const {
    vector<RGB> pixels(height * width);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            RGB accumulatedColor(0, 0, 0);
            for (int i = 0; i < samples; i++) {
                float u = static_cast<float>(x) - (width / 2) + static_cast<float>(rand()) / RAND_MAX;
                float v = static_cast<float>(y) - (height / 2) + static_cast<float>(rand()) / RAND_MAX;
                Ray ray = generateRay(u, v);
                accumulatedColor += traceRay(ray, scene);
            }
            pixels[y * width + x] = accumulatedColor / samples;
        }
    }

    return Image(width, height, pixels);
}

Ray PinholeCamera::generateRay(float x, float y) const {
    Direction direction = (left * x + up * y - forward).normalize();
    return Ray(origin, direction);
}

RGB PinholeCamera::traceRay(const Ray& ray, const Scene& scene) const {
    auto intersection = scene.intersect(ray);
    if (intersection.has_value()) {
        return intersection->material;
    }
    return RGB(0, 0, 0);
}
