#include "../include/object3D.hpp"
#include <vector>
#include <fstream>

using namespace std;

vector<RGB> PinholeCamera::render(const Scene& scene) const {
    vector<RGB> image(height * width);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            RGB accumulatedColor(0, 0, 0);
            for (int i = 0; i < samples; ++i) {
                Ray ray = generateRay(x, y);
                accumulatedColor = accumulatedColor + traceRay(ray, scene);
            }
            image[y * width + x] = accumulatedColor / samples;
        }
    }

    return image;
}

Ray PinholeCamera::generateRay(int x, int y) const {
    float aspectRatio = static_cast<float>(width) / height;
    float px = (2 * ((x + 0.5) / width) - 1) * aspectRatio;
    float py = (1 - 2 * ((y + 0.5) / height));

    Direction direction = (left * px + up * py - forward).normalize();
    return Ray(origin, direction);

}

// TODO: Coger la emisión de la intersección
RGB PinholeCamera::traceRay(const Ray& ray, const Scene& scene) const {
    
    auto intersection = scene.intersect(ray);
    if (intersection.has_value()) {
        return RGB(1, 1, 1);
    }
    return RGB(0, 0, 0);

}
