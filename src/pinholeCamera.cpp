#include "../include/pinholeCamera.hpp"
#include "../include/object3D.hpp"
#include "../include/Image.hpp"
#include "../include/parallel_renderer.hpp"
#include "../include/rendering_strategy.hpp"
#include "../include/utils.hpp"
#include "../include/scene.hpp"
#include "../include/constants.hpp"
#include "../include/toneMapping.hpp"
#include <vector>
#include <fstream>
#include <random>
#include <cmath>

/********************
 * Métodos Públicos *
 ********************/

PinholeCamera::PinholeCamera(const Point& origin, const int FOV, const int width, const int height, const Direction& forward) 
    : origin(origin), forward(forward), width(width), height(height) {
    if (!(width > 0 && height > 0 && FOV > 0 && FOV < 180 && forward.mod() > 0.0f)) {
        throw std::invalid_argument("Invalid camera parameters: width, height, FOV must be positive and forward vector must be non-zero.");
    }
    float aspectRatio = static_cast<float>(width) / height;
    float halfFOV = tan(FOV * 0.5 * (M_PI / 180)); // Convert FOV to radians and then take the tangent
    halfExtentX = halfFOV;
    halfExtentY = halfExtentX / aspectRatio;

    Direction worldUp{0, 1, 0}; // Default up direction in world coordinates (down because PPM writes row 0 at the top)
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
    calculatePixelSizes(); // Calculate pixel sizes based on extents
}

PinholeCamera::PinholeCamera(const Point& origin, const Direction& up, const Direction& left, const Direction& forward, int width, int height)
                            : origin(origin), left(left), up(up), forward(forward), width(width), height(height), halfExtentX(left.mod()), halfExtentY(up.mod())
    { calculatePixelSizes(); }

// Main unified render method
Image PinholeCamera::render(const Scene& scene, const RenderConfig& config) const {
    std::vector<RGB> pixels(height * width);
    Image image;
    if (config.mode == RenderingMode::PARALLEL) {
        image = ParallelRenderer::render(*this, scene, config);
    } else {
        renderRegion(pixels, scene, config); // Renders whole image
        image = Image(width, height, pixels);
    }
        
    if (config.toneMapping != ToneMappingType::NONE)
        ToneMapping::apply(image, config);
    
    return image;
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
            
            showProgressIfNeeded(config.verbose); // Show progress if verbose mode is enabled
        }
    }
}

RGB PinholeCamera::traceRay(const Ray& ray, const Scene& scene) const {
    // Find the closest intersection of the ray with the scene
    auto intersection = scene.intersect(ray);

    if (!intersection)
        return scene.backgroundColor; // No intersection, return background color
    
    // Si no hay luces en la escena, devolvemos el color del material
    int lightAmount = scene.lights.size();
    if (lightAmount == 0)
        return intersection->material.diffuse;
    
    return scene.calculateDirectLight(*intersection); // Return the color of the material at the intersection point
}


RGB PinholeCamera::tracePath(const Ray& ray, const Scene& scene, unsigned depth) const {
    
    if (depth > 10) // Caso base: Máximo número de rebotes
        return RGB(0, 0, 0);

    auto intersection = scene.intersect(ray);
    if (!intersection)
        return scene.backgroundColor;

    const Material& mat = intersection->material;
    if (mat.isEmissive())
        return mat.emission;

    const Direction& normal = intersection->normal;
    const Point& hitP = intersection->point;

    // Russian roulette weights
    float pd = mat.p_diffuse;
    float ps = mat.p_specular;
    float pt = mat.p_transmittance;
    float sum = pd + ps + pt;

    float r = rand0_1() * (sum > 0 ? sum : 1.0f);

    if (r < pd) {
        // Diffuse lobe
        RGB direct = scene.calculateDirectLight(*intersection);
        Direction wi = randomCosineDirection(normal);
        float cosTheta = utils::cosTheta(normal, wi);
        Ray newRay(hitP + normal * EPSILON, wi);
        RGB indirect = tracePath(newRay, scene, depth + 1);
        RGB f = mat.diffuse * (1.0f / M_PI);
        return direct + indirect * f * cosTheta / pd;

    } else if (r < pd + ps) {
        // Specular lobe
        Direction refl = (ray.direction - normal * 2 * ray.direction.dot(normal)).normalize();
        Ray newRay(hitP + normal * EPSILON, refl);
        RGB ret = tracePath(newRay, scene, depth + 1);
        float cosTheta = utils::cosTheta(normal, refl);
        RGB f = mat.specular / cosTheta;
        return ret * f / ps;

    } else if (r < pd + ps + pt) {
        // Direction transmit = mat.refractar(ray.direction, normal).normalize();
        // // choose offset opposite normal
        // Ray newRay(hitP - normal * EPSILON, transmit);
        // RGB ret = tracePath(newRay, scene, depth + 1);
        // float cosTheta = utils::cosTheta(normal, transmit);
        // RGB f = mat.transmittance / cosTheta;
        // return ret * f / pt;
        // Transmission lobe
        auto transmitOpt = mat.refractar(ray.direction, normal);
        
        // Check if refraction was successful (not total internal reflection)
        if (!transmitOpt.has_value()) {
            // Total internal reflection - treat as specular reflection
            Direction refl = (ray.direction - normal * 2 * ray.direction.dot(normal)).normalize();
            Ray newRay(hitP + normal * EPSILON, refl);
            RGB ret = tracePath(newRay, scene, depth + 1);
            return ret * mat.specular / pt; // Use pt since we're in transmission branch
        }

        Direction transmit = transmitOpt->normalize();

        // For transmission, we need to offset in the direction of transmission
        // If ray is entering (normal pointing towards ray), offset inward
        // If ray is exiting (normal pointing away from ray), offset outward
        float dotProduct = ray.direction.dot(normal);
        Direction offset = (dotProduct < 0) ? -normal * EPSILON : normal * EPSILON;
        
        Ray newRay(hitP + offset, transmit);
        RGB ret = tracePath(newRay, scene, depth + 1);
        
        // Simple transmission without Fresnel for now
        return ret * mat.transmittance / pt;
    } else // absorved
        return RGB(0, 0, 0);
}

void PinholeCamera::showProgressIfNeeded(bool verbose) const {
    static std::atomic<size_t> renderedPixelCount{0};
    static std::atomic<int> lastProgressShown{-1};

    if (!verbose) return;
    renderedPixelCount.fetch_add(1); // Increment

    size_t total = static_cast<size_t>(width) * height;
    int currentProgress = total > 0 ? static_cast<float>(renderedPixelCount.load()) / total * 100.0f : 0.0f;
    int lastShown = lastProgressShown.load();
    
    if (currentProgress > lastShown && 
        lastProgressShown.compare_exchange_strong(lastShown, currentProgress)) {
        
        std::cout << '\r' << '[';
        int barWidth = 50;
        int pos = barWidth * currentProgress / 100;
        
        for (int i = 0; i < barWidth; ++i) {
            if (i < pos) std::cout << '=';
            else if (i == pos) std::cout << '>';
            else std::cout << ' ';
        }
        
        std::cout << "] " << currentProgress << "%";
        if (currentProgress >= 100) {
            std::cout << std::endl;
            lastProgressShown.store(-1); // Reset for next render
            renderedPixelCount.store(0); // Reset counter at start of render
        } else {
            std::cout.flush();
        }
    }
}

