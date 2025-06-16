#include "../include/pinholeCamera.hpp"
#include "../include/object3D.hpp"
#include "../include/Image.hpp"
#include "../include/parallel_renderer.hpp"
#include "../include/rendering_strategy.hpp"
#include "../include/utils.hpp"
#include "../include/scene.hpp"
#include "../include/constants.hpp"
#include <vector>
#include <fstream>
#include <random>
#include <cmath>

/********************
 * Métodos Públicos *
 ********************/

PinholeCamera::PinholeCamera(const Point& origin, const int FOV, const int width, const int height) 
: origin(origin), width(width), height(height) {

    float aspectRatio = static_cast<float>(width) / height;
    float halfFOV = tan(FOV * 0.5 * (M_PI / 180)); // Convert FOV to radians and then take the tangent
    halfExtentX = halfFOV;
    halfExtentY = halfExtentX / aspectRatio;

    left = Direction(-1, 0, 0) * halfExtentX;
    up = Direction(0, 1, 0) * halfExtentY;
    forward = Direction(0, 0, 1);
    
    calculatePixelSizes();
}

// Main unified render method
Image PinholeCamera::render(const Scene& scene, unsigned samplesPerPixel, 
                           const RenderConfig& config) const {
    auto strategy = StrategyFactory::createStrategy(config.algorithm);
    if (config.mode == RenderingMode::PARALLEL) {
        ParallelRenderer renderer(config);
        return renderer.render(*this, scene, samplesPerPixel, config);
    } else {
        std::vector<RGB> pixels(height * width);
        for (int y = 0; y < height; y++) {
            float normalizedY = ((static_cast<float>(y) - (height / 2.0f)) / (height / 2.0f)) * halfExtentY;
            for (int x = 0; x < width; x++) {
                float normalizedX = ((static_cast<float>(x) - (width / 2.0f)) / (width / 2.0f)) * halfExtentX;
                RGB pixelColor = strategy->calculatePixelColor(*this, scene, normalizedX, normalizedY, samplesPerPixel, config);
                pixels[y * width + x] = pixelColor;
            }
        }
        // pixels[width * (height/2) + width/2] = RGB(1, 0, 0); // Mark the center pixel as red for debugging
        return Image(width, height, pixels);
    }
}


RGB PinholeCamera::traceRay(const Ray& ray, const Scene& scene) const {
    // Find the closest intersection of the ray with the scene
    auto intersection = scene.intersect(ray);

    // Return the color of the intersected material, or black if no intersection
    RGB color = RGB(0, 0, 0);
    if (intersection) {

        int lightAmount = scene.lights.size();

        // Si no hay luces en la escena, devolvemos el color del material
        if (lightAmount == 0)
            return intersection->material.diffuse;

        // Iteramos por cada una de las luces de la escena
        for (int i = 0; i < lightAmount; i++) {
 
            PointLight* currentLight = dynamic_cast<PointLight*>(scene.lights[i].get());
            if (!currentLight) continue; // Skip if not a point light

            Direction lightVector = (currentLight->center - intersection->point);
            float lightDistance = lightVector.mod();
            Direction lightDirection = lightVector.normalize();

            // Shadow ray with proper origin offset along normal
            Point shadowOrigin = intersection->point + intersection->normal * EPSILON; // Offset to avoid self-intersection
            Ray shadowRay(shadowOrigin, lightDirection);
            auto obstruction = scene.intersect(shadowRay, lightDistance);

            if (obstruction)
                continue;

            RGB powerByDistance = currentLight->light / (lightDistance * lightDistance);
            RGB brdf = intersection->material.diffuse * (1.0f / M_PI); // Lambertian reflectance
            Direction normal = intersection->normal.normalize();
            float cosTheta = std::max(0.0f, normal.dot(lightDirection));

            color += powerByDistance * brdf * cosTheta; 
            
        }
        return color; // Return the color of the material at the intersection point
    } else {
        return scene.backgroundColor; // No intersection, return background color
    }
}



RGB PinholeCamera::tracePath(const Ray& ray, const Scene& scene, unsigned depth) const {
    
    if (depth > 20) // Caso base: Máximo número de rebotes
        return RGB(0, 0, 0);

    // Se intersecta el rayo con la escena
    // Si no hay intersección, devolvemos el color de fondo
    std::optional<Intersection> intersection = scene.intersect(ray);
    if (!intersection) {
        return scene.backgroundColor;
    }

    // Si el material es emisivo, devolvemos su color (como una fuente de luz)
    if (intersection->material.isEmissive) {
        return intersection->material.diffuse;
    }

    // Cálculo de la luz directa
    RGB directLight(0, 0, 0);
    RGB indirectLight(1, 1, 1);
    float diffuse = intersection->material.diffuse.max();
    float specular = intersection->material.specular.max();

    if (diffuse + specular > 0.9f) {
        diffuse = 0.9f * diffuse / (diffuse + specular);
        specular = 0.9f * specular / (diffuse + specular);
    }

    float randomValue = rand0_1();
    if (randomValue < diffuse) {
        // Si el valor aleatorio es menor que la probabilidad de difuso, devolvemos la luz directa
        directLight = scene.calculateDirectLight(intersection->point);
        indirectLight = indirectLight * (intersection->material.diffuse / diffuse); // Difuso
    } else if (randomValue < diffuse + specular) {
        // Si el valor aleatorio está entre la probabilidad de difuso y especular, devolvemos el color especular
        
        // TODO: No se para que se usa esta variable
        Direction wr = (ray.direction - intersection->normal * 2 * ray.direction.dot(intersection->normal)).normalize();
        (void)wr; // Suppress unused variable warning
        indirectLight = indirectLight * (intersection->material.specular / specular); // Especular
    } else {
        return scene.backgroundColor; // Matamos el rayo
    }

    // Rebote indirecto: dirección aleatoria en el hemisferio de la normal
    Direction randomDir = randomCosineDirection(intersection->normal);
    Ray randomRay(intersection->point + randomDir * EPSILON, randomDir);

    // Ruleta rusa para terminar caminos largos
    float survivalProbability = std::min(0.9f, intersection->material.diffuse.max());
    if (depth >= 3 && rand0_1() > survivalProbability) {
        return directLight;
    }

    // Recursión para el rebote indirecto
    RGB reflectedColor = tracePath(randomRay, scene, depth + 1);
    if (depth >= 3) {
        reflectedColor = reflectedColor / survivalProbability;
    }

    float cosTheta = std::max(0.0f, intersection->normal.dot(randomDir));
    RGB brdf = intersection->material.diffuse * (1.0f / M_PI);

    // Suma de luz directa e indirecta
    return directLight * brdf * cosTheta + reflectedColor;
}
