#include "../include/object3D.hpp"
#include "../include/Image.hpp"
#include <vector>
#include <fstream>
#include <random>
#include <cmath>

using namespace std;

/********************
 * Métodos Públicos *
 ********************/

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

// TODO: Por algún motivo, no funcionan las intersecciones con los planos
Image PinholeCamera::renderRayTracing(const Scene& scene, unsigned samplesPerPixel) const {
    vector<RGB> pixels(height * width);

    for (int y = 0; y < height; y++) {
        float normalizedY = static_cast<float>(y) - (height / 2);
        for (int x = 0; x < width; x++) {
            float normalizedX = static_cast<float>(x) - (width / 2);
            // Calculate the color of the pixel at (x, y)
            RGB pixelColor = calculatePixelColorRayTracing(scene, normalizedX, normalizedY, samplesPerPixel);
            pixels[y * height + x] = pixelColor;
        }
    }

    return Image(width, height, pixels);
}

Image PinholeCamera::renderPhotonMapping(const Scene& scene, unsigned samplesPerPixel, 
                MapaFotones mapa, unsigned kFotones, double radio, Kernel* kernel) const {
    
    vector<RGB> pixels(height * width);
    
    for (int y = 0; y < height; y++) {
        float normalizedY = static_cast<float>(y) - (height / 2);
        for (int x = 0; x < width; x++) {
            float normalizedX = static_cast<float>(x) - (width / 2);
            RGB pixelColor = calculatePixelColorPhotonMapping(scene, normalizedX, normalizedY, samplesPerPixel);
            pixels[y * width + x] = pixelColor;
        }
    }
    return Image(width, height, pixels);
}

/********************
 * Métodos Privados *
 ********************/

Ray PinholeCamera::generateRay(float x, float y) const {
    // Calculate the direction of the ray
    Direction direction = (left * x + up * y + forward);
    return Ray(origin, direction.normalize());
}

RGB PinholeCamera::calculatePixelColorPathTracing(const Scene& scene, float x, float y, unsigned samplesPerPixel) const {

    RGB accumulatedColor(0, 0, 0);

    for (unsigned i = 0; i < samplesPerPixel; i++) { // Example for image (100x100), x = 0, y = 0, samplesPerPixel = 1
        // Generate a random offset for anti-aliasing
        float x_offset = x + rand0_1(); // -50.5
        float y_offset = y + rand0_1(); // -50.5

        // Generate a ray through the pixel
        Ray ray = generateRay(x_offset, y_offset);

        // Trace the ray and accumulate the color
        accumulatedColor += tracePath(ray, scene);
    }

    // Average the accumulated color
    return accumulatedColor / samplesPerPixel;
}

RGB PinholeCamera::calculatePixelColorRayTracing(const Scene& scene, float x, float y, unsigned samplesPerPixel) const {

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

RGB PinholeCamera::calculatePixelColorPhotonMapping(const Scene& scene, float x, float y, unsigned samplesPerPixel) const {

    RGB accumulatedColor(0, 0, 0);

    for (unsigned i = 0; i < samplesPerPixel; i++) { // Example for image (100x100), x = 0, y = 0, samplesPerPixel = 1
        // Generate a random offset for anti-aliasing
        float x_offset = x + rand0_1(); // -50.5
        float y_offset = y + rand0_1(); // -50.5

        // Generate a ray through the pixel
        Ray ray = generateRay(x_offset, y_offset);
        optional<Intersection> interseccion = scene.intersect(ray);
        if (interseccion) {
            // Esta es la única linea interesante, es la siguiente función
            accumulatedColor = accumulatedColor + scene.ecuacionRenderFotones(interseccion->point, r.d, interseccion.geometria, 
                interseccion->normal, mapa, kFotones, radio, guardar, kernel);
        }
    }

    // Average the accumulated color
    return accumulatedColor / samplesPerPixel;
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
            return intersection->material.diffuse;
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

            RGB brdf = intersection->material.diffuse * (1.0f / M_PI); // Lambertian reflectance

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

/*
 * Generates a random direction on the hemisphere defined by the normal vector.
 * This is used for path tracing to sample directions uniformly.
 */
// https://the-last-stand.github.io/ray-tracing-practice/the_rest_of_your_life/generating_random_directions/
Direction randomCosineDirection(const Direction& normal) {
    float r1 = 2 * M_PI * rand0_1();
    float r2 = rand0_1();
    float r2s = sqrt(r2);

    // Base ortonormal
    Direction w = normal;
    // Vectores perpendiculares a w
    Direction u = ((fabs(w.x) > 0.1 ? Direction(0,1,0) : Direction(1,0,0)).cross(w)).normalize();
    Direction v = w.cross(u);

    return (u * cos(r1) * r2s + v * sin(r1) * r2s + w * sqrt(1 - r2)).normalize();
}

RGB PinholeCamera::tracePath(const Ray& ray, const Scene& scene, unsigned depth) const {
    
    if (depth > 20) { // Caso base: Máximo número de rebotes
        return RGB(0, 0, 0);
    }

    // Se intersecta el rayo con la escena
    // Si no hay intersección, devolvemos el color de fondo
    optional<Intersection> intersection = scene.intersect(ray);
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
        indirectLight = indirectLight * (intersection->material.specular / specular); // Especular
    } else {
        return scene.backgroundColor; // Matamos el rayo
    }

    // Rebote indirecto: dirección aleatoria en el hemisferio de la normal
    Direction randomDir = randomCosineDirection(intersection->normal);
    Ray randomRay(intersection->point + randomDir * EPSILON, randomDir);

    // Ruleta rusa para terminar caminos largos
    float survivalProbability = min(0.9f, intersection->material.diffuse.max());
    if (depth >= 3 && rand0_1() > survivalProbability) {
        return directLight;
    }

    // Recursión para el rebote indirecto
    RGB reflectedColor = tracePath(randomRay, scene, depth + 1);
    if (depth >= 3) {
        reflectedColor = reflectedColor / survivalProbability;
    }

    float cosTheta = max(0.0f, intersection->normal.dot(randomDir));
    RGB brdf = intersection->material.diffuse * (1.0f / M_PI);

    // Suma de luz directa e indirecta
    return directLight * brdf * cosTheta + reflectedColor;
}
