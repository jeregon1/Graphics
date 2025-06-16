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

PinholeCamera::PinholeCamera(const Point& origin, const int FOV, const int width, const int height, const Direction& forward) {
    if (!(width > 0 && height > 0 && FOV > 0 && FOV < 180 && forward.mod() > 0.0f)) {
        throw std::invalid_argument("Invalid camera parameters: width, height, FOV must be positive and forward vector must be non-zero.");
    }
    float aspectRatio = static_cast<float>(width) / height;
    float halfFOV = tan(FOV * 0.5 * (M_PI / 180)); // Convert FOV to radians and then take the tangent
    halfExtentX = halfFOV;
    halfExtentY = halfExtentX / aspectRatio;

    Direction worldUp{0, 1, 0}; // Default up direction in world coordinates
    // Calculate the right axis as the cross product of forward and world up
    Direction right = forward.cross(worldUp);
    // If the forward vector is collinear with world up, we need to choose a different up vector
    if (right.mod() < EPSILON) {
        worldUp = Direction(0, 0, 1); // Alternative up vector
        right = forward.cross(worldUp);
    }
    
    // Calculate the up vector as the cross product of right and forward
    Direction up_normalized = right.cross(forward).normalize();

    up = up_normalized * halfExtentY; // Up vector scaled by half extent in Y
    left = -right.normalize() * halfExtentX; // Left vector is the negative of right scaled by half extent in X

    *this = PinholeCamera(origin, up, left, forward, width, height);
}

PinholeCamera::PinholeCamera(const Point& origin, const Direction& up, const Direction& left, const Direction& forward, int width, int height)
                            : origin(origin), left(left), up(up), forward(forward), width(width), height(height), halfExtentX(left.mod()), halfExtentY(up.mod())
    { calculatePixelSizes(); }

// Main unified render method
Image PinholeCamera::render(const Scene& scene, const RenderConfig& config) const {
    std::vector<RGB> pixels(height * width);
    if (config.mode == RenderingMode::PARALLEL) {
        return ParallelRenderer::render(*this, scene, config);
    } else {
        renderRegion(pixels, scene, config); // Renders whole image
        return Image(width, height, pixels);
    }
}

void PinholeCamera::renderRegion(std::vector<RGB>& pixels, const Scene& scene, const RenderConfig& config, 
                                 int startY, int startX, int endY, int endX) const {
    endY = (endY == -1) ? height : endY; // Whole column if endY is -1
    endX = (endX == -1) ?  width : endX; // Whole row if endX is -1
    
    auto strategy = StrategyFactory::createStrategy(config.algorithm);
    for (int y = startY; y < endY; y++) {
        
        // Normalize Y coordinate to the range [-1, 1] so that later ray generation works correctly
        // 1. (y + 0.5) centers the sample in the pixel
        // 2. Subtract (height / 2.0f) to center at image middle in the range [-height/2, height/2]
        // 3. Divide by (height / 2.0f) to normalize to [-1, 1] range
        float normalizedY = ((static_cast<float>(y) + 0.5f - (height / 2.0f)) / (height / 2.0f));
        for (int x = startX; x < endX; x++) {
            float normalizedX = ((static_cast<float>(x) + 0.5f - (width / 2.0f)) / (width / 2.0f)); // Same logic for X
            
            pixels[y * width + x] = strategy->calculatePixelColor(*this, scene, normalizedX, normalizedY, config);
        }
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
