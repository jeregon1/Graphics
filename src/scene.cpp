#include "../include/pinholeCamera.hpp"
#include "../include/object3D.hpp"
#include "../include/scene.hpp"
#include "../include/constants.hpp"
#include "../include/utils.hpp"

#include <vector>
#include <memory>
#include <optional>
#include <list>
#include <string>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <tuple>
#include <iostream>

using namespace std;

/*********
 * Scene *
 *********/

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

RGB Scene::calculateDirectLight(const Intersection& inter) const {
    RGB color = RGB(0, 0, 0);
    for (const auto& light : lights) {
        Direction lightToObjectDir = (light->center - inter.point);
        float lightToObjectDistance = lightToObjectDir.mod();
        
        // Check if surface faces the light
        lightToObjectDir = lightToObjectDir.normalize();
        float cosTheta = utils::cosTheta(inter.normal, lightToObjectDir);
        if (cosTheta <= 0.0f) 
            continue; // Surface faces away from light

        // Shadow ray with proper origin offset along normal
        Point shadowOrigin = inter.point + inter.normal * EPSILON; // Offset to avoid self-intersection
        Ray shadowRay(shadowOrigin, lightToObjectDir);

        bool obstruction = intersectAny(shadowRay, lightToObjectDistance);
        if (obstruction)
            continue;

        RGB powerByDistance = light->power / (lightToObjectDistance * lightToObjectDistance);
        // RGB brdf = inter.material.diffuse * / M_PI; // Lambertian reflectance
        RGB brdf = inter.material.evaluateBSDF(lightToObjectDir, -shadowRay.direction, inter.normal);
        color += powerByDistance * brdf * cosTheta; 
    }
    return color;
}


MapaFotones Scene::generarMapaFotones(int nPaths, bool save, double sigma) const {
    list<Foton> fotones;
    double totalEmision = 0.0;
    for (const auto& light : lights) totalEmision += light->power.max(); // Obtiene el total de emisión de todas las luces
    for (const auto& light : lights) {
        int numFotones = (int)(nPaths*light->power.max()/totalEmision); // Distribuye los paseos de fotones según la emisión de cada luz
        for (int j = 0; j < numFotones; j++) {
            Direction d = muestraAleatoriaUniforme(); // Muestra una dirección aleatoria en el ángulo sólido
            Ray r = Ray(light->center, d);
            RGB lightColor = light->power / numFotones; // Distribución uniforme de la luz
            reboteFoton(r, RGB(lightColor.r*4*M_PI, lightColor.g*4*M_PI, lightColor.b*4*M_PI), fotones, fotones, save, sigma);
        }
    }
    MapaFotones mapa = construirMapaFotones(fotones);
    return mapa;  
}

// Acceleration structure management methods
void Scene::buildAccelerationStructure(AccelerationStructure type) {
    currentAcceleration_ = type;
    accelerationStructure_ = AccelerationStructureFactory::create(type);
    accelerationStructure_->build(objects);
    accelerationBuilt_ = true;
}

bool Scene::intersectAny(const Ray& ray, const float distance) const {
    // Use acceleration structure if available
    if (accelerationStructure_ && accelerationBuilt_) {
        return accelerationStructure_->intersectAny(ray, distance);
    }
    
    // Fallback to linear search
    for (const auto& object : objects) {
        auto intersection = object->intersect(ray);
        if (intersection && intersection->distance < distance) {
            return true;
        }
    }
    return false;
}

// TODO: Revisar que funcione el código
// Estas dos imágenes generan una lista de fotones en la escena
void Scene::reboteFoton(const Ray& ray, const RGB& light, list<Foton>& fotones, 
            list<Foton>& causticos, bool esCaustico, bool save, double sigma) const {
    
    (void)save; // Suppress unused parameter warning
    
    // Variable initialization
    auto intersection = this->intersect(ray);
    bool primerRebote = true;

    if (!intersection) { // If no intersection, do nothing
        return;
    }

    Direction wo = ray.direction;
    Direction wi;

    double norma = (ray.origin - intersection->point).mod();
    norma = norma * norma;
    RGB brdf = light/norma;
    
    do { // If it intersects the scene
        // Si interseca con una luz de área, guardamos el fotón
        /*
        if (i.geometria->esLuzArea()) {
            // Si se pone guardar, se guarda el fotón
            if (guardar) {
                Foton f = Foton(i.punto, wo, radiancia);
                fotones.push_back(f);
            }
            return;
        }
        */

        Material material = intersection->material;
        double probability = rand0_1(); // Probabilidad aleatoria entre 0 y 1
        
        // Difuso
        if (probability <= material.p_diffuse) { 
            Direction normal = intersection->normal;
            if (ray.direction * normal > 0.0) {
                normal = Direction(-normal.x, -normal.y, -normal.z); // Dirección del rayo * normal de la intersección
            } 
            Foton f = Foton(intersection->point, wo, brdf);
            if (!primerRebote) {
                if (esCaustico) {
                    causticos.push_back(f);
                } else {
                    fotones.push_back(f);
                }
            }
            
            esCaustico = false;
            wi = muestraAleatoriaUniforme(); // Obtención de una dirección aleatoria de la hemiesfera
            brdf = brdf * abs(wi * normal) * material.diffuse/material.p_diffuse; // BRDF difuso 
        } 
        
        // Specular
        else if (probability <= material.p_diffuse + material.p_specular) { 
            esCaustico = true;
            Direction normal = intersection->normal;
            if (ray.direction * normal > 0.0) {
                normal = Direction(-normal.x, -normal.y, -normal.z); // Direction of ray * intersection normal
            } 
            // Perfect reflection: R = I - 2(I·N)N
            wi = wo - normal * (2.0f * (wo * normal));
            brdf = brdf * abs(wi * normal) * material.specular/material.p_specular; // Specular BRDF
        } 
        
        // Refracción
        else if (probability <= material.p_diffuse + material.p_specular + material.p_transmittance) { 
            esCaustico = true;
            Direction normal = intersection->normal;
            wi = *material.refractar(wo, normal); // Funcion brdf
            brdf = brdf * abs(wi * normal) * material.diffuse/material.p_transmittance; // BRDF de refracción
        }

        // Absorción
        else if (probability > material.p_diffuse + material.p_specular + material.p_transmittance) {
            // Si no se cumple ninguna de las condiciones anteriores, no hacemos nada
            return;
        }
        
        // Sigma se refiere a la atenuación de la luz, que se puede usar para simular la dispersión de la luz en el medio
        norma = (ray.origin - intersection->point).mod();
        norma = norma * norma;
        brdf = brdf * pow(M_E, -sigma*norma);
        primerRebote = false;

    } while ((intersection = this->intersect(Ray(intersection->point, wi))));
}

// TODO: Refactorizar nombres de variables y funciones
RGB Scene::ecuacionRenderFotones(Point point, Direction wo, Material material, Direction normal, 
    MapaFotones mapa, int kFotones, double radio, bool guardar, Kernel* kernel, double sigma) const {
    
    // Caso base
    if (material.isEmissive()) {
        return material.diffuse;
    } 

    double radioFotonMasLejano = 0.0;
    double radioFoton = 0.0;
    Point posFoton;
    RGB L = RGB(0, 0, 0);

    double probability = rand0_1(); // Probabilidad aleatoria entre 0 y 1

    // Seguimos hasta llegar a una superficie difusa, simulando el camino del foton
    while (probability <= material.p_diffuse + material.p_specular + material.p_transmittance) {

        if (probability <= material.p_diffuse) {
            if (wo * normal > 0.0) {
                normal = Direction(-normal.x, -normal.y, -normal.z); // Dirección del rayo * normal de la intersección
            } 
            wo = wo - normal * 2.0f * (wo * normal); // Ecuación de reflexión
        } else { // Especular
            wo = *material.refractar(wo, normal); // Ecuación de refracción
        }

        // Se maneja siguiente intersección
        auto intersection = this->intersect(Ray(point, wo));
        if (!intersection) {
            return L; // Si no hay intersección, se devuelve la luz acumulada
        } else {
            point = intersection->point;
            material = intersection->material;
            normal = intersection->normal;
            probability = rand0_1();
        }
    }

    if (probability <= material.p_diffuse) {

        if (wo * normal > 0.0) {
            normal = Direction(-normal.x, -normal.y, -normal.z); // Dirección del rayo * normal de la intersección
        } 

        // Obtener fotones cercanos con radio r y máximo k
        // Función nearest_neighbors de la clase MapaFotones proporcionada por los profesores
        vector<const Foton*> fotones = mapa.nearest_neighbors(point, kFotones, radio);
        
        // Se obtiene el foton más lejano
        for (const Foton* foton : fotones) {
            posFoton = foton->posicion;
            radioFoton = (posFoton - point).mod();
            if (radioFoton > radioFotonMasLejano) radioFotonMasLejano = radioFoton;
        }
        for (const Foton* f : fotones) {
            Direction wi = f->direccion;
            double coseno = Direction(-normal.x, -normal.y, -normal.z) * wi;
            if (coseno > 0.0) {
                posFoton = f->posicion;
                L += (material.diffuse / material.p_diffuse) * f->flujo
                    *kernel->evaluar((posFoton - point).mod(), radioFotonMasLejano);
            }
        }
        // Estimacion de la luz directa
        if (!guardar) L += estimacionSiguienteEvento(point, wo, material, normal, sigma);
    }
    return L;
}

// Devuelve la luz directa en un punto de la escena sobre una geometria difusa
RGB Scene::estimacionSiguienteEvento(Point point, Direction wo, Material material, Direction n, double sigma) const {
    
    (void)wo; // Suppress unused parameter warning
    
    RGB L = RGB(0, 0, 0);
    // Recorremos todas las luces puntuales
    // y calculamos la luz directa que llega al punto x
    // con la BRDF de Lambert
    for (size_t i = 0; i < lights.size(); i++) {
        Direction wi = (lights[i]->center - point).normalize();
        double norma = (lights[i]->center - point).mod();
        norma = norma * norma; // Norma al cuadrado
        double coseno = n * wi;
        RGB fr = material.diffuse / M_PI; // BRDF Lambertiano
        if (coseno > 0) {
            auto interseccion = this->intersect(Ray(lights[i]->center, Direction(-wi.x, -wi.y, -wi.z)));
            if (interseccion && interseccion->distance >= sqrt(norma) - EPSILON) {
                if (sigma == 0.0) L = L + (fr * coseno) * (lights[i]->power / norma);
                else L = L + (fr * coseno) * (lights[i]->power / norma) * pow(M_E, -sigma * norma);
            }
        }
    }
    return L;
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
        s.addObject(std::make_shared<Plane>(Direction(0, 0, 1), whiteDiffuse, 1));

        // Spheres
        s.addObject(std::make_shared<Sphere>(Point(-0.5, -0.7, 0.25), 0.3, magentaDiffuse));
        s.addObject(std::make_shared<Sphere>(Point(0.5, -0.7, -0.25), 0.3, blueDiffuse));

        // Point light at ceiling center
        s.addLight(std::make_shared<PointLight>(Point(0, 0.5, 0), RGB(10, 10, 10)));

        return s;
    }();
    
    return scene;
}

// YAML parsing helper functions
namespace {
    Material parseMaterial(std::istringstream& iss, std::ifstream& file) {
        
        // Try to read simple format first (RGB values)
        float r, g, b;
        if (iss >> r >> g >> b) {
            return Material(RGB(r,g,b));
        }
        
        Material material;
        // Complex format with properties
        std::string line;
        while (std::getline(file, line)) {
            size_t first = line.find_first_not_of(" \t");
            if (first == std::string::npos || line[first] == '#') continue;
            
            // Check if this line belongs to another section
            if (first == 0) {
                // Put the line back by seeking
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
                    material.diffuse = RGB(dr, dg, db);
                }
            } else if (property == "specular:") {
                float sr, sg, sb;
                if (lineStream >> sr >> sg >> sb) {
                    material.specular = RGB(sr, sg, sb);
                }
            } else if (property == "transmittance:") {
                float tr, tg, tb;
                if (lineStream >> tr >> tg >> tb) {
                    material.transmittance = RGB(tr, tg, tb);
                }
            } else if (property == "emission:") {
                float er, eg, eb;
                if (lineStream >> er >> eg >> eb) {
                    material.emission = RGB(er, eg, eb);
                }
            }
        }
        material.normalize(); // Normalize the material properties
        return material;
    }
    
    PinholeCamera parseCamera(std::ifstream& file) {
        Point origin(0, 0, -3);
        Direction left(-1, 0, 0);
        Direction up(0, 1, 0);
        Direction forward(0, 0, 1);
        int width = 512, height = 512;
        int fov = -1; // -1 indicates FOV not set
        
        std::string line;
        while (std::getline(file, line)) {
            size_t first = line.find_first_not_of(" \t");
            if (first == std::string::npos || line[first] == '#') continue;
            
            // Check if this line belongs to another section
            if (first == 0) {
                // Put the line back by seeking
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
                if (lineStream >> fov) {
                    // FOV value read successfully
                }
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
                if (lineStream >> width >> height) {
                    // Successfully read both values
                }
            }
        }
        
        // If FOV is set, use FOV constructor
        if (fov != -1)
            return PinholeCamera(origin, fov, width, height, forward);
        else
            return PinholeCamera(origin, up, left, forward, width, height);
    }
}

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
        
        // std::cout << "DEBUG: Processing keyword: '" << keyword << "' from line: '" << line << "'" << std::endl;
        
        if (keyword == "background:") {
            float r, g, b;
            iss >> r >> g >> b;
            scene.backgroundColor = RGB(r, g, b);
            // std::cout << "DEBUG: Set background to " << r << " " << g << " " << b << std::endl;
        }
        else if (keyword == "camera:") {
            camera = parseCamera(file);
            // std::cout << "DEBUG: Parsed camera" << std::endl;
        }
        else if (keyword == "material:") {
            currentMaterial = parseMaterial(iss, file);
            // std::cout << "DEBUG: Parsed material: " << currentMaterial.toString() << std::endl;
        }
        else if (keyword == "sphere:") {
            float x, y, z, radius;
            iss >> x >> y >> z >> radius;
            scene.addObject(std::make_shared<Sphere>(Point(x, y, z), radius, currentMaterial));
            // std::cout << "DEBUG: Added sphere at (" << x << ", " << y << ", " << z << ") with radius " << radius << std::endl;
        }
        else if (keyword == "plane:") {
            float nx, ny, nz, d;
            iss >> nx >> ny >> nz >> d;
            scene.addObject(std::make_shared<Plane>(Direction(nx, ny, nz), currentMaterial, (int)d));
            // std::cout << "DEBUG: Added plane with normal (" << nx << ", " << ny << ", " << nz << ") at distance " << d << std::endl;
        }
        else if (keyword == "light:") {
            float x, y, z, lr, lg, lb;
            iss >> x >> y >> z >> lr >> lg >> lb;
            scene.addLight(std::make_shared<PointLight>(Point(x, y, z), RGB(lr, lg, lb)));
            // std::cout << "DEBUG: Added light at (" << x << ", " << y << ", " << z << ") with color (" << lr << ", " << lg << ", " << lb << ")" << std::endl;
        }
        else {
            // std::cout << "DEBUG: Unknown keyword: '" << keyword << "'" << std::endl;
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
        // Note: Camera internal vectors are private, so we output basic info
        // For full camera reconstruction, we'd need accessor methods
        file << "\n";
    }

    // Materials and objects
    Material lastMaterial;
    bool firstObject = true;
    
    for (const auto& object : objects) {
        // Check if we need to output a new material
        const Material& objMaterial = object->getMaterial();
        if (firstObject || !(objMaterial == lastMaterial)) {
            file << "material: " << objMaterial.diffuse.r << " " << objMaterial.diffuse.g << " " << objMaterial.diffuse.b << "\n";
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
        // Add other object types as needed
        
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
