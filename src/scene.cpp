#include "pinholeCamera.hpp"
#include "object3D.hpp"
#include "scene.hpp"
#include "constants.hpp"
#include "utils.hpp"

#include <vector>
#include <memory>
#include <optional>
#include <list>
#include <string>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <tuple>
#include <cmath>  // For std::sqrt
#include <iostream>

using namespace std;


optional<Intersection> Scene::intersect(const Ray& ray, const float distance) const {
    // Use acceleration structure if available
    if (accelerationStructure_ && accelerationBuilt_) {
        return accelerationStructure_->intersect(ray, distance);
    }
    
    // Fallback to linear search
    optional<Intersection> closest_intersection = nullopt;
    for (const auto& object : objects) {
        auto intersection = object->intersect(ray);
        if (intersection && (!closest_intersection || intersection->distance < closest_intersection->distance) && intersection->distance < distance) {
            closest_intersection = intersection;
        }
    }
    return closest_intersection;
}

bool Scene::intersectAny(const Ray& ray, const float distance) const {
    // Use acceleration structure if available
    if (accelerationStructure_ && accelerationBuilt_) {
        return accelerationStructure_->intersectAny(ray, distance);
    }
    
    // Fallback to linear search
    for (const auto& object : objects) {
        auto intersection = object->intersect(ray);
        if (intersection && intersection->distance < distance)
            return true;
    }
    return false;
}

RGB Scene::nextEventEstimation(const Intersection& inter) const {
    if (getLightCount() == 0)
        return inter.material.getDiffuse(); // No lights in the scene
    
    RGB color = RGB(0, 0, 0);
    
    // Sample point lights
    for (const auto& light : lights) {
        Direction lightToObjectDir = (light->center - inter.point);
        float lightToObjectDistance = lightToObjectDir.mod();
        
        // Check if surface faces the light
        lightToObjectDir = lightToObjectDir.normalize();
        float cosTheta = utils::cosTheta(inter.normal, lightToObjectDir);
        if (cosTheta <= 0) 
            continue; // Surface faces away from light

        // Shadow ray with proper origin offset along normal
        Point shadowOrigin = inter.point + inter.normal * EPS; // Offset to avoid self-intersection
        Ray shadowRay(shadowOrigin, lightToObjectDir);

        bool obstruction = intersectAny(shadowRay, lightToObjectDistance);
        if (obstruction)
            continue;

        RGB powerByDistance = light->power / (lightToObjectDistance * lightToObjectDistance);
        
        RGB brdf = inter.material.evaluateBSDF(lightToObjectDir, -shadowRay.direction, inter.normal);
        color += powerByDistance * brdf * cosTheta; 
    }
    
    // Sample area lights (emissive objects with finite area)
    for (const auto& object : objects) {
        if (!object->getMaterial().isEmissive() || object->area() <= 0.0f)
            continue;
        
        // Sample a point on the area light
        auto lightSample = object->sampleLightPoint();
        if (!lightSample.has_value())
            continue;
        
        const auto& ls = lightSample.value();
        
        // Direction from surface to light sample
        Direction toLight = ls.position - inter.point;
        float dist = toLight.mod();
        toLight = toLight.normalize();
        
        // Check if surface faces the light
        float cosSurface = inter.normal.dot(toLight);
        if (cosSurface <= 0.0f)
            continue;
        
        // Check if light surface faces the intersection point
        float cosLight = ls.normal.dot(-toLight);
        if (cosLight <= 0.0f)
            continue;
        
        // Cast shadow ray
        Point shadowOrigin = inter.point + inter.normal * EPS;
        Ray shadowRay(shadowOrigin, toLight);
        
        bool obstruction = intersectAny(shadowRay, dist - EPS);
        if (obstruction)
            continue;
        
        // Calculate geometric term: (cos_surface * cos_light) / dist^2
        float dist2 = dist * dist;
        float geometricTerm = (cosSurface * cosLight) / (dist2 * ls.pdf);
        
        // Evaluate BRDF
        RGB brdf = inter.material.evaluateBSDF(toLight, -shadowRay.direction, inter.normal);
        
        // Add contribution from area light
        color += ls.emission * brdf * geometricTerm;
    }
    
    return color;
}

void Scene::generarMapaFotones(const int nPaths, unsigned maxBounces) {
    list<Foton> fotones;      // Regular diffuse photons
    list<Foton> causticos;    // Caustic photons (specular/transmissive → diffuse)
    
    // Calculate total light emission for energy distribution
    double totalEmision = std::accumulate(lights.begin(), lights.end(), 0.0, 
        [](double sum, const std::shared_ptr<PointLight>& light) {
            return sum + light->getPowerSum();
        });
    
    for (const auto& light : lights) {
        // Distribute photons proportionally to light power
        int numFotones = (int)(nPaths * light->getPowerSum() / totalEmision);

        for (int j = 0; j < numFotones; j++) {
            Direction d = muestraAleatoriaUniforme();
            Ray r = Ray(light->center, d);
            // Photon flux = 4π * p0 / S (conservation of energy)
            RGB photonFlux = light->power * (4 * M_PI) / numFotones;
            reboteFoton(r, photonFlux, fotones, causticos, false, maxBounces);
        }
    }

    if (fotones.empty() && causticos.empty()) {
        std::cout << "Warning: No photons generated!" << std::endl;
        mapaFotones_ = construirMapaFotones(std::list<Foton>());
        mapaCausticos_ = construirMapaFotones(std::list<Foton>());
        photonMapBuilt_ = true;
        return;
    }
    
    std::cout << "Generated " << fotones.size() << " regular photons and " 
              << causticos.size() << " caustic photons" << std::endl;

    // Build separate photon maps
    mapaFotones_   = construirMapaFotones(fotones);
    mapaCausticos_ = construirMapaFotones(causticos);
    photonMapBuilt_ = true;
}

// Photon random walk following professor's specifications
void Scene::reboteFoton(Ray currentRay,
                       RGB currentFlux,
                       list<Foton>& fotones,
                       list<Foton>& causticos,
                       bool esCaustico,
                       unsigned maxBounces) const {

    bool firstBounce = true;
    for (unsigned bounce = 0; bounce < maxBounces; bounce++) {
        auto intersection = this->intersect(currentRay);
        if (!intersection)
            return; // Photon escaped scene
        
        Material material = intersection->material;
        Direction normal = intersection->normal;
        
        // Ensure normal faces incoming ray
        if (currentRay.direction.dot(normal) > 0.0)
            normal = -normal;
        
        // Russian roulette for material interaction
        double totalProb = material.p_diffuse + material.p_specular + material.p_transmittance;
        
        if (totalProb <= 0.0)
            return; // No interaction possible
        
        double probability = rand0_1() * totalProb;  // Normalize probabilities
        
        Direction newDirection;
        bool storePhoton = false;
        RGB radiance;

        if (probability <= material.p_diffuse) { // Diffuse interaction - this is where we store photons
            
            newDirection = randomCosineDirection(normal);
            RGB brdfWeight = material.evaluateBSDF(newDirection, -currentRay.direction, normal);
            storePhoton = true;
            radiance = brdfWeight * utils::cosTheta(normal, newDirection);
            // Don't change caustic flag - if it was caustic, it stays caustic
        }
        else if (probability <= material.p_diffuse + material.p_specular) {
            // Specular reflection - continue tracing, don't store photon
            newDirection = currentRay.direction.specular(normal);
            radiance = material.getSpecular();
            esCaustico = true; // Mark as caustic path
        }
        else if (probability <= material.p_diffuse + material.p_specular + material.p_transmittance) {
            // Transmission/refraction - continue tracing, don't store photon
            auto refracted = material.refract(currentRay.direction, normal);
            if (refracted) {
                newDirection = *refracted;
                radiance = material.getTransmittance();
                esCaustico = true; // Mark as caustic path
            } else
                return; // Total internal reflection, absorb photon
        }
        else {
            return; // Absorption - photon is absorbed
        }
        
        // Store photon only on diffuse surfaces and NOT on first bounce
        if (storePhoton && !firstBounce) {
            // Store with incoming light direction
            Foton f(intersection->point, currentRay.direction, currentFlux);
            if (esCaustico)
                causticos.push_back(f);
            else
                fotones.push_back(f);
        }
        
        // Update photon flux
        currentFlux *= radiance;
        
        // Create new ray for next bounce (offset to avoid self-intersection)
        Point newOrigin = intersection->point + newDirection * EPS;
        currentRay = Ray(newOrigin, newDirection);
        
        firstBounce = false;
        
        // Russian roulette termination based on flux intensity
        if (bounce > 5) {
            double maxFlux = currentFlux.max();
            if (maxFlux < 0.01 || rand0_1() > maxFlux)
                break;
        }
    }
}

// Photon mapping radiance estimation following professor's specifications
RGB Scene::ecuacionRenderFotones(Direction wo, const Intersection& intersection,
    const RenderConfig& config, const Kernel& kernel, const int bouncesLeft) const {

    if (bouncesLeft < 0)
        return RGB(0, 0, 0); // No more bounces left

    Material material = intersection.material; // Ensure material is defined from intersection

    // Base case: emissive materials
    if (material.isEmissive())
        return material.emission;

    // Russian roulette for BSDF sampling
    float pd = material.p_diffuse;
    float ps = material.p_specular;
    float pt = material.p_transmittance;
    float sum = pd + ps + pt;
    if (sum <= 0.0f)
        return RGB(0, 0, 0);

    float r = rand0_1() * sum;

    // Diffuse branch: photon density estimation + next event
    if (r < pd) {
        RGB Ld(0, 0, 0);
        Direction n = intersection.normal;
        if (wo.dot(n) > 0.0f)
            n = -n;

        // Gather nearby regular photons
        auto photons = mapaFotones_.nearest_neighbors(
            intersection.point,
            config.kPhotons,
            config.radius
        );
        
        // Gather nearby caustic photons (with higher importance)
        auto causticPhotons = mapaCausticos_.nearest_neighbors(
            intersection.point,
            config.kPhotons,  // Use fewer caustic photons since they're more important
            config.radius
        );

        RGB regularContrib(0, 0, 0);
        RGB causticContrib(0, 0, 0);

        // Process regular photons
        for (auto f : photons) {
            Direction wi = -f->incidentDir;
            double cosT = utils::cosTheta(n, wi);
            if (cosT > 0.0) {
                double d = (f->position - intersection.point).mod();
                double w = kernel.evaluar(d, config.radius);
                RGB brdf = material.evaluateBSDF(wi, -wo, n); // BRDF for diffuse reflection
                regularContrib += brdf * f->flux * cosT * w;
            }
        }

        // Process caustic photons with higher weight (3x importance)
        const float causticWeight = 3.0f;
        for (auto f : causticPhotons) {
            Direction wi = -f->incidentDir;
            double cosT = utils::cosTheta(n, wi);
            if (cosT > 0.0) {
                double d = (f->position - intersection.point).mod();
                double w = kernel.evaluar(d, config.radius);
                RGB brdf = material.evaluateBSDF(wi, -wo, n); // BRDF for diffuse reflection
                causticContrib += brdf * f->flux * cosT * w * causticWeight;
            }
        }

        // Normalize by search radius area
        double area = M_PI * config.radius * config.radius;
        Ld = (regularContrib + causticContrib) / area;

        // Always add direct lighting
        RGB direct = nextEventEstimation(intersection);

        return (Ld + direct) / pd;
    }
    // Specular branch
    else if (r < pd + ps) {
        Direction dir = wo.specular(intersection.normal);
        Point orig = intersection.point + dir * EPS;
        if (auto hit = this->intersect(Ray(orig, dir))) {
            RGB Li = ecuacionRenderFotones(dir, *hit, config, kernel, bouncesLeft - 1);
            return Li * material.getSpecular() / ps;
        }
    }
    // Transmission branch
    else if (r < pd + ps + pt) {
        auto refr = material.refract(wo, intersection.normal);
        if (!refr)
            return RGB(0, 0, 0);
        Direction dir = *refr;
        Point orig = intersection.point + dir * EPS;
        if (auto hit = this->intersect(Ray(orig, dir))) {
            RGB Li = ecuacionRenderFotones(dir, *hit, config, kernel, bouncesLeft - 1);
            return Li * material.getTransmittance() / pt;
        }
    }
    return RGB(0, 0, 0);
}


string Scene::toString() const {
    ostringstream oss;
    oss << "=== SCENE ===\n";
    oss << "Background: RGB(" << backgroundColor.r << ", " << backgroundColor.g << ", " << backgroundColor.b << ")\n\n";
    
    oss << "Objects (" << objects.size() << "):\n";
    for (const auto& object : objects) {
        oss << object->toString() << "\n\n";
    }
    
    oss << "\nLights (" << lights.size() << "):\n";
    for (const auto& light : lights) {
        oss << light->toString() << "\n\n";
    }
    
    return oss.str();
}

// Cornell box default scene
Scene& Scene::defaultScene() {
    static Scene scene = []() {
        Scene s(RGB(0.2f, 0.2f, 0.2f)); // Default background color

        // White diffuse material with small specular component (plastic-like)
        Material whiteDiffuse = Material::createPlastic(RGB(0.8f, 0.8f, 0.8f));

        // Red diffuse for left wall (purely diffuse)
        Material redDiffuse = Material::createPurelyDiffuse(RGB(1, 0, 0));

        // Green diffuse for right wall (purely diffuse)
        Material greenDiffuse = Material::createPurelyDiffuse(RGB(0, 1, 0));

        // Magenta for left sphere (plastic)
        Material magentaDiffuse = Material::createPlastic(RGB(1, 0, 1));

        // Blue for right sphere (dielectric glass-like)
        Material blueDiffuse = Material::createDielectric(1.5);

        // Floor (y = -1)
        s.addObject(std::make_shared<Plane>(Direction(0, -1, 0), whiteDiffuse, 1));
        // Ceiling (y = 1)
        s.addObject(std::make_shared<Plane>(Direction(0, 1, 0), whiteDiffuse, 1));
        // Left wall (x = -1)
        s.addObject(std::make_shared<Plane>(Direction(-1, 0, 0), redDiffuse, 1));
        // Right wall (x = 1)
        s.addObject(std::make_shared<Plane>(Direction(1, 0, 0), greenDiffuse, 1));
        // Back wall (z = -3)
        s.addObject(std::make_shared<Plane>(Direction(0, 0, -1), whiteDiffuse, 3));

        // Magenta sphere on left
        s.addObject(std::make_shared<Sphere>(Point(-0.5, -0.7, -0.25), 0.3, magentaDiffuse));

        // Blue sphere on right
        s.addObject(std::make_shared<Sphere>(Point(0.5, -0.7, -0.25), 0.3, blueDiffuse));

        // Point light at ceiling center
        s.addLight(std::make_shared<PointLight>(Point(0, 0.5, 0), RGB(10, 10, 10)));

        return s;
    }();
    
    return scene;
}

auto parseMaterial = [](std::istringstream& iss, std::ifstream& file) -> Material {
    // Try to read simple format first (RGB values)
    float r, g, b;
    if (iss >> r >> g >> b) {
        return Material(RGB(r,g,b));
    }
    Material material;
    std::string line;
    while (std::getline(file, line)) {
        size_t first = line.find_first_not_of(" \t");
        if (first == std::string::npos || line[first] == '#') continue;
        if (first == 0) {
            file.seekg(-(long)line.length() - 1, std::ios::cur);
            break;
        }
        line = line.substr(first);
        std::istringstream lineStream(line);
        std::string property;
        lineStream >> property;
        if (property == "diffuse:") {
            float dr, dg, db;
            if (lineStream >> dr >> dg >> db) {
                material.setDiffuse(RGB(dr, dg, db));
            }
        } else if (property == "specular:") {
            float s;
            if (lineStream >> s) {
                material.setSpecular(RGB(s, s, s));
            }
        } else if (property == "transmittance:") {
            float t;
            if (lineStream >> t) {
                material.setTransmittance(RGB(t, t, t));
            }
        } else if (property == "emission:") {
            float er, eg, eb;
            if (lineStream >> er >> eg >> eb) {
                material.emission = RGB(er, eg, eb);
            }
        } else if (property == "n:") {
            float ni;
            if (lineStream >> ni)
                material.n = ni;
        }
    }
    return material;
};

auto parseCamera = [](std::ifstream& file) -> PinholeCamera {
    Point origin(0, 0, -3);
    Direction left(-1, 0, 0);
    Direction up(0, 1, 0);
    Direction forward(0, 0, 1);
    int width = 512, height = 512;
    int fov = -1;
    std::string line;
    while (std::getline(file, line)) {
        size_t first = line.find_first_not_of(" \t");
        if (first == std::string::npos || line[first] == '#') continue;
        if (first == 0) {
            file.seekg(-(long)line.length() - 1, std::ios::cur);
            break;
        }
        line = line.substr(first);
        std::istringstream lineStream(line);
        std::string property;
        lineStream >> property;
        if (property == "origin:") {
            float x, y, z;
            if (lineStream >> x >> y >> z) {
                origin = Point(x, y, z);
            }
        } else if (property == "fov:") {
            if (lineStream >> fov) {}
        } else if (property == "left:") {
            float x, y, z;
            if (lineStream >> x >> y >> z) {
                left = Direction(x, y, z);
            }
        } else if (property == "up:") {
            float x, y, z;
            if (lineStream >> x >> y >> z) {
                up = Direction(x, y, z);
            }
        } else if (property == "forward:") {
            float x, y, z;
            if (lineStream >> x >> y >> z) {
                forward = Direction(x, y, z);
            }
        } else if (property == "pixels:") {
            if (lineStream >> width >> height) {}
        }
    }
    if (fov != -1)
        return PinholeCamera(origin, fov, width, height, forward);
    else
        return PinholeCamera(origin, up, left, forward, width, height);
};

auto parseConeOrCylinder = [](std::ifstream& file, Point& base, Direction& axis, float& radius, float& height) {
    std::string line;
    while (std::getline(file, line)) {
        size_t first = line.find_first_not_of(" \t");
        if (first == std::string::npos || line[first] == '#') continue;
        if (first == 0) {
            file.seekg(-(long)line.length() - 1, std::ios::cur);
            break;
        }
        line = line.substr(first);
        std::istringstream lineStream(line);
        std::string property;
        lineStream >> property;
        if (property == "base:") {
            float x, y, z;
            if (lineStream >> x >> y >> z) base = Point(x, y, z);
        } else if (property == "axis:") {
            float x, y, z;
            if (lineStream >> x >> y >> z) axis = Direction(x, y, z);
        } else if (property == "radius:") {
            lineStream >> radius;
        } else if (property == "height:") {
            lineStream >> height;
        }
    }
};

// YAML scene loader - returns scene and optional camera
std::optional<std::pair<Scene, std::optional<PinholeCamera>>> Scene::fromYAML(const std::string& filename) {
    Scene scene;
    std::optional<PinholeCamera> camera = std::nullopt;
    
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return std::nullopt;
    }
    std::string line;
    Material currentMaterial;
    
    while (std::getline(file, line)) {
        // Trim whitespace
        size_t first = line.find_first_not_of(" \t\r\n");
        if (first == std::string::npos || line[first] == '#') continue;
        line = line.substr(first);
        if (line.empty()) continue;
        
        std::istringstream iss(line);
        std::string keyword;
        iss >> keyword;
        
        
        if (keyword == "background:") {
            float r, g, b;
            iss >> r >> g >> b;
            scene.backgroundColor = RGB(r, g, b);
        }
        else if (keyword == "camera:") {
            camera = parseCamera(file);
        }
        else if (keyword == "material:") {
            currentMaterial = parseMaterial(iss, file);
        }
        else if (keyword == "sphere:") {
            float x, y, z, radius;
            iss >> x >> y >> z >> radius;
            scene.addObject(std::make_shared<Sphere>(Point(x, y, z), radius, currentMaterial));
        }
        else if (keyword == "plane:") {
            float nx, ny, nz, d;
            iss >> nx >> ny >> nz >> d;
            scene.addObject(std::make_shared<Plane>(Direction(nx, ny, nz), currentMaterial, d));
        }
        else if (keyword == "light:") {
            float x, y, z, lr, lg, lb;
            iss >> x >> y >> z >> lr >> lg >> lb;
            scene.addLight(std::make_shared<PointLight>(Point(x, y, z), RGB(lr, lg, lb)));
        }
        else if (keyword == "triangle:") {
            float ax, ay, az, bx, by, bz, cx, cy, cz;
            iss >> ax >> ay >> az >> bx >> by >> bz >> cx >> cy >> cz;
            scene.addObject(std::make_shared<Triangle>(
                Point(ax, ay, az),
                Point(bx, by, bz),
                Point(cx, cy, cz),
                currentMaterial
            ));
        }
        else if (keyword == "quad:") {
            float cx, cy, cz, ux, uy, uz, vx, vy, vz;
            iss >> cx >> cy >> cz >> ux >> uy >> uz >> vx >> vy >> vz;
            scene.addObject(std::make_shared<Quad>(
                Point(cx, cy, cz),
                Direction(ux, uy, uz),
                Direction(vx, vy, vz),
                currentMaterial
            ));
        }
        else if (keyword == "cone:" || keyword == "cylinder:") {
            Point base;
            Direction axis;
            float radius = 1, height = 1;
            parseConeOrCylinder(file, base, axis, radius, height);
            if (keyword == "cone:")
                scene.addObject(std::make_shared<Cone>(base, axis, radius, height, currentMaterial));
            else
                scene.addObject(std::make_shared<Cylinder>(base, axis, radius, height, currentMaterial));
        }
    }
    
    return std::make_pair(std::move(scene), camera);
}

bool Scene::saveToYAML(const std::string& filename, const PinholeCamera* camera) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file for writing: " << filename << std::endl;
        return false;
    }
    
    file << "# Scene Configuration\n";
    file << "# Generated automatically\n\n";
    
    // Background
    file << "background: " << backgroundColor.r << " " << backgroundColor.g << " " << backgroundColor.b << "\n\n";
    
    // Camera (if provided)
    if (camera) {
        file << "camera:\n";
        Point origin = camera->getOrigin();
        file << "  origin: " << origin.x << " " << origin.y << " " << origin.z << "\n";
        file << "  fov: " << camera->getFOV() << "\n";
        file << "  forward: " << camera->getForward().x << " " << camera->getForward().y << " " << camera->getForward().z << "\n";
        file << "  pixels: " << camera->getWidth() << " " << camera->getHeight() << "\n";
        file << "\n";
    }

    // Materials and objects
    Material lastMaterial;
    bool firstObject = true;
    
    for (const auto& object : objects) {
        // Check if we need to output a new material
        const Material& objMaterial = object->getMaterial();
        if (firstObject || !(objMaterial == lastMaterial)) {
            // Output extended material block
            file << "material:\n";
            file << "  diffuse: "
                 << objMaterial.getDiffuse().r << " "
                 << objMaterial.getDiffuse().g << " "
                 << objMaterial.getDiffuse().b << "\n";
            if (objMaterial.getSpecular().max() > EPS)
                file << "  specular: " << objMaterial.getSpecular().r << "\n";
            if (objMaterial.getTransmittance().max() > EPS)
                file << "  transmittance: " << objMaterial.getTransmittance().r << "\n";
            if (objMaterial.emission.max() > EPS)
                file << "  emission: "
                     << objMaterial.emission.r << " "
                     << objMaterial.emission.g << " "
                     << objMaterial.emission.b << "\n";
            if (objMaterial.n != 1.0)
                file << "  n: " << objMaterial.n << "\n";
            lastMaterial = objMaterial;
        }
        
        // Output object based on type
        if (auto sphere = std::dynamic_pointer_cast<Sphere>(object)) {
            Point center = sphere->center;
            float radius = sphere->radius;
            file << "sphere: " << center.x << " " << center.y << " " << center.z << " " << radius << "\n";
        }
        else if (auto plane = std::dynamic_pointer_cast<Plane>(object)) {
            Direction normal = plane->normal;
            int distance = plane->distance;
            file << "plane: " << normal.x << " " << normal.y << " " << normal.z << " " << distance << "\n";
        }
        else if (auto triangle = std::dynamic_pointer_cast<Triangle>(object)) {
            Point a = triangle->a;
            Point b = triangle->b; 
            Point c = triangle->c;
            file << "triangle: " << a.x << " " << a.y << " " << a.z << " "
                 << b.x << " " << b.y << " " << b.z << " "
                 << c.x << " " << c.y << " " << c.z << "\n";
        }
        else if (auto quad = std::dynamic_pointer_cast<Quad>(object)) {
            Point center = quad->center;
            Direction u = quad->u;
            Direction v = quad->v;
            file << "quad: " << center.x << " " << center.y << " " << center.z << " "
                 << u.x << " " << u.y << " " << u.z << " "
                 << v.x << " " << v.y << " " << v.z << "\n";
        }
        // Add other object types as needed
        else if (auto cone = std::dynamic_pointer_cast<Cone>(object)) {
            Point base = cone->base;
            Direction axis = cone->axis;
            float radius = cone->radius;
            float height = cone->height;
            file << "cone: base: " << base.x << " " << base.y << " " << base.z << "\n";
            file << "     axis: " << axis.x << " " << axis.y << " " << axis.z << "\n";
            file << "     radius: " << radius << "\n";
            file << "     height: " << height << "\n";
        }
        else if (auto cylinder = std::dynamic_pointer_cast<Cylinder>(object)) {
            Point base = cylinder->base;
            Direction axis = cylinder->axis;
            float radius = cylinder->radius;
            float height = cylinder->height;
            file << "cylinder: base: " << base.x << " " << base.y << " " << base.z << "\n";
            file << "       axis: " << axis.x << " " << axis.y << " " << axis.z << "\n";
            file << "       radius: " << radius << "\n";
            file << "       height: " << height << "\n";
        }
        
        firstObject = false;
    }
    
    if (!objects.empty()) file << "\n";
    
    // Lights
    for (const auto& light : lights) {
        Point pos = light->center;
        RGB color = light->power;
        file << "light: " << pos.x << " " << pos.y << " " << pos.z << " " 
             << color.r << " " << color.g << " " << color.b << "\n";
    }
    file.close();
    
    if (camera)
        std::cout << "Scene with camera saved to: " << filename << std::endl;
    else
        std::cout << "Scene saved to: " << filename << std::endl;

    return true;
}

void Scene::buildAccelerationStructure(AccelerationStructure type) {
    currentAcceleration_ = type;
    accelerationBuilt_ = false;
    
    if (type == AccelerationStructure::NONE) {
        accelerationStructure_.reset();
        return;
    }
    
    accelerationStructure_ = AccelerationStructureFactory::create(type);
    if (accelerationStructure_) {
        accelerationStructure_->build(objects);
        accelerationBuilt_ = true;
        std::cout << accelerationStructure_->getStats() << std::endl;
    }
}
