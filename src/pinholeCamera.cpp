#include "../include/object3D.hpp"
#include "../include/Image.hpp"
#include <vector>
#include <fstream>
#include <random>
#include <cmath>

using namespace std;

#define EPISILON 0.05

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

        // Si no hay luces en la escena, devolvemos el color del material
        if (lightAmount == 0) {
            return intersection->material;
        }

        // Iteramos por cada una de las luces de la escena
        for (int i = 0; i < lightAmount; i++) {

            PointLight* currentLight = dynamic_cast<PointLight*>(scene.lights[i].get());

            // TODO: Comprobar si la dirección es el sentido correcto
            Direction obstructionDirection = (currentLight->center - intersection->point); // Dirección desde el punto de intersección hasta la luz
            float obstructionDistance = obstructionDirection.mod(); // Evitamos colisiones con otros objetos más lejanos que la luz
            Ray obstructionRay(intersection->point * (1+EPSILON), obstructionDirection); // Creamos el raycast para comprobar la colisión
            auto obstruction = scene.intersect(obstructionRay, obstructionDistance);

            if (obstruction) {
                continue;
            }

            RGB powerByDistance = currentLight->light / pow(Direction(currentLight->center - intersection->point).mod(), 2);

            RGB brdf = intersection->material * (1.0f / M_PI); // Lambertian reflectance

            Direction lightDirection = (currentLight->center - intersection->point).normalize();
            Direction normal = intersection->normal.normalize();
            float cosTheta = max(float(0.0), normal.dot(lightDirection));

            color += powerByDistance * brdf * cosTheta; 
            
        }
        return color; // Return the color of the material at the intersection point
    } else {
        return scene.backgroundColor; // No intersection, return background color
    }
}
