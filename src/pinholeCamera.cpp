#include "../include/object3D.hpp"
#include "../include/Image.hpp"
#include <vector>
#include <fstream>
#include <random>

using namespace std;

PinholeCamera::PinholeCamera(const Point& origin, const int FOV, const int width, const int height) 
: origin(origin), width(width), height(height) {

    float aspectRatio = static_cast<float>(width) / height;
    float halfFOV = tan(FOV * 0.5 * (M_PI / 180)); // Convert FOV to radians and then take the tangent
    float halfWidth = halfFOV;
    float halfHeight = halfWidth / aspectRatio;

    left = Direction(-1, 0, 0) * halfWidth;
    up = Direction(0, 1, 0) * halfHeight;
    forward = Direction(0, 0, 3); 
}


Image PinholeCamera::render(const Scene& scene, unsigned samplesPerPixel) const {
    vector<RGB> pixels(height * width);

    for (int y = 0; y < height; y++) {
        float normalizedY = static_cast<float>(y) - (height / 2);
        for (int x = 0; x < width; x++) {
            float normalizedX = static_cast<float>(x) - (width / 2);
            // Calculate the color of the pixel at (x, y)
            RGB pixelColor = calculatePixelColor(scene, normalizedX, normalizedY, samplesPerPixel);
            pixels[y * height + x] = pixelColor;
        }
    }

    return Image(width, height, pixels);
}

RGB PinholeCamera::calculatePixelColor(const Scene& scene, float x, float y, unsigned samplesPerPixel) const {

    RGB accumulatedColor(0, 0, 0);

    for (unsigned i = 0; i < samplesPerPixel; i++) { // Example for image (100x100), x = 0, y = 0, samplesPerPixel = 1
        // Generate a random offset for anti-aliasing
        float x_offset = x + rand0_1(); // -50.5
        float y_offset = y + rand0_1(); // -50.5

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
    Direction direction = (left * x + up * y + forward);
    return Ray(origin, direction.normalize());
}

RGB PinholeCamera::traceRay(const Ray& ray, const Scene& scene) const {
    // Find the closest intersection of the ray with the scene
    auto intersection = scene.intersect(ray);

    // Return the color of the intersected material, or black if no intersection
    RGB color = RGB(0, 0, 0);
    if (intersection) {

        int lightAmount = scene.lights.size();

        if (lightAmount == 0) {
            return intersection->material; // No lights, return material color
        }

        for (int i = 0; i < lightAmount; i++) {

            // TODO: Comprobar si la intersección está en la sombra de la luz, no iterar si no hay luz
            Direction obstructionDirection = (scene.lights[i]->center - intersection->point);
            float obstructionDistance = obstructionDirection.mod();
            Ray obstructionRay(intersection->point, obstructionDirection);
            auto obstruction = scene.intersect(obstructionRay, obstructionDistance); // Check for obstruction
            if (obstruction) {
                continue; // If there's an obstruction, skip this light
            }

            PointLight* currentLight = dynamic_cast<PointLight*>(scene.lights[i].get());
            
            RGB powerByDistance = currentLight->light / Direction(currentLight->center - intersection->point).mod();
            
            RGB brdf = intersection->material * max(0.0f, intersection->normal.dot((currentLight->center - intersection->point).normalize()));

            Direction lightDirection = (currentLight->center - intersection->point).normalize();
            Direction normal = intersection->normal.normalize();
            float cosTheta = abs(normal.dot(lightDirection));

            color += powerByDistance * brdf * cosTheta; 
            
        }
        return color; // Return the color of the material at the intersection point
    } else {
        return scene.backgroundColor; // No intersection, return background color
    }
}
