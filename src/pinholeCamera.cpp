#include "../include/object3D.hpp"
#include <vector>
#include <fstream>

using namespace std;

vector<RGB> PinholeCamera::render(const Scene& scene) const {
    vector<RGB> image(height * width);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            RGB accumulatedColor(0, 0, 0);
            for (int i = 0; i < samples; i++) {
                float u = static_cast<float>(x) + static_cast<float>(rand()) / RAND_MAX;
                float v = static_cast<float>(y) + static_cast<float>(rand()) / RAND_MAX;
                Ray ray = generateRay(u, v);
                accumulatedColor = accumulatedColor + traceRay(ray, scene);
            }
            image[y * width + x] = accumulatedColor / samples;
        }
    }

    return image;
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
