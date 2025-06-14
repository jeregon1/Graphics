#include "../include/object3D.hpp"

#include <vector>

/*******
 * Ray *
 *******/

Point Ray::at(float t) const {
    return direction * t + origin;
}

/*********
 * Scene *
 *********/

void Scene::addObject(const shared_ptr<Object3D>& object) {
    objects.push_back(object);
}

void Scene::addLight(const shared_ptr<PointLight>& light) {
    lights.push_back(light);
}

optional<Intersection> Scene::intersect(const Ray& ray, const float distance) const {
    optional<Intersection> closest_intersection = nullopt;
    for (const auto& object : objects) {
        auto intersection = object->intersect(ray);
        if (intersection && (!closest_intersection || intersection->distance < closest_intersection->distance) && intersection->distance < distance) {
            closest_intersection = intersection;
        }
    }
    return closest_intersection;
}

RGB Scene::calculateDirectLight(const Point& p) const {

    RGB directLight(0, 0, 0);
    int lightAmount = lights.size();

    for (int i = 0; i < lightAmount; i++) {
        PointLight* currentLight = dynamic_cast<PointLight*>(lights[i].get()); // Se obtiene el puntero
        Direction lightDirection = currentLight->center - p;
        float distanceToLight = lightDirection.mod(); // Primero calculamos la distancia a la luz
        if (distanceToLight < EPSILON) {
            continue; // Si la distancia es muy pequeña, no consideramos la luz
        }

        // Create a ray from the point to the light
        Ray lightRay(p + lightDirection * EPSILON, lightDirection.normalize());
        optional<Intersection> obstruction = intersect(lightRay, distanceToLight);

        if (!obstruction) { // Si no hay obstaculos, consideramos la luz
            RGB powerByDistance = currentLight->light / (distanceToLight * distanceToLight);
            directLight += powerByDistance;
        }
        
    }

    return directLight; // Lambertian reflectance

}

MapaFotones Scene::generarMapaFotones(int nPaths, bool save, double sigma) {
    list<Foton> fotones;
    double totalEmision = 0.0;
    for (const auto& light : lights) totalEmision += light->light.max(); // Obtiene el total de emisión de todas las luces
    for (const auto& light : lights) {
        int numFotones = (int)(nPaths*light->light.max()/totalEmision); // Distribuye los paseos de fotones según la emisión de cada luz
        for (int j = 0; j < numFotones; j++) {
            Direction d = muestraAleatoriaUniforme(); // Muestra una dirección aleatoria en el ángulo sólido
            Ray r = Ray(light->center, d);
            RGB lightColor = light->light / numFotones; // Distribución uniforme de la luz
            reboteFoton(r, RGB(lightColor.r*4*M_PI, lightColor.g*4*M_PI, lightColor.b*4*M_PI), fotones, fotones, save, sigma);
        }
    }
    MapaFotones mapa = construirMapaFotones(fotones);
    return mapa;  
}

// TODO: Revisar que funcione el código
// Estas dos imágenes generan una lista de fotones en la escena
void Scene::reboteFoton(const Ray& ray, const RGB& light, std::list<Foton>& fotones, 
            std::list<Foton>& causticos, bool esCaustico, bool save, double sigma) {
    
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
        double probability = (double) rand0_1(); // Probabilidad aleatoria entre 0 y 1
        
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
        else if (probability <= material.p_diffuse + material.p_specular + material.p_transparency) { 
            esCaustico = true;
            Direction normal = intersection->normal;
            wi = material.refractar(wo, normal); // Funcion brdf
            brdf = brdf * abs(wi * normal) * material.diffuse/material.p_transparency; // BRDF de refracción
        }

        // Absorción
        else if (probability > material.p_diffuse + material.p_specular + material.p_transparency) {
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

/*
// TODO: Complete photon mapping implementation - this function needs to be rewritten in English
// RGB Scene::ecuacionRenderFotones(Point x, Direction wo, Object3D* geo, Direction n, 
//             MapaFotones mapa, int kFotones, double radio, bool guardar, Kernel* kernel, double sigma) {
//     // Implementation needs to be completed
//     return RGB(0, 0, 0);
// }

// RGB Scene::estimacionSiguienteEvento(Point x, Direction wo, Object3D* g, Direction n, double sigma) {
//     // Implementation needs to be completed  
//     return RGB(0, 0, 0);
// }
*/

string Scene::toString() const {
    string result;
    for (const auto& object : objects) {
        result += object->toString() + "\n";
    }
    return result;
}
 
/**********
 * Sphere *
 **********/

string Sphere::toString() const {
    ostringstream oss;
    oss << "Center: " << center << "\n"
        << "Radius: " << radius;
    return oss.str();
}

/*
Source: https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-sphere-intersection.html
*/
optional<Intersection> Sphere::intersect(const Ray& r) const {
    
    Direction oc = center - r.origin;
    float tca = oc.dot(r.direction);

    if (tca < 0) {
        return nullopt;
    }

    float d2 = oc.dot(oc) - tca * tca;

    if (d2 > radius * radius) { 
        return nullopt;
    }

    float thc = sqrt(radius * radius - d2);
    float t0 = tca - thc;
    float t1 = tca + thc;

    if (t0 < 0 && t1 < 0) {
        return nullopt;
    }

    float t = (t0 < t1) ? t0 : t1;

    if (t < 0) {
        t = (t0 > t1) ? t0 : t1;
        if (t < 0) {
            return nullopt;
        }
    }

    return Intersection(t, r.at(t), (r.at(t) - center).normalize(), material);
    
    /*
    Direction oc = center - r.origin;
    float a = r.direction.dot(r.direction);
    float b = 2.0f * oc.dot(r.direction);
    float c = oc.dot(oc) - radius * radius;
    float discriminant = b * b - 4 * a * c;

    if (discriminant < 0) {
        return nullopt;
    } else {
        float t1 = (-b - sqrt(discriminant)) / (2.0f * a);
        float t2 = (-b + sqrt(discriminant)) / (2.0f * a);
        
        // If both intersection points are behind the ray origin, no intersection
        if (t1 < 0 && t2 < 0) return nullopt;

        float t;
        // If one intersection point is behind the ray origin, return the other
        if (t1 < 0) t = t2;
        if (t2 < 0) t = t1;

        // Return the closest intersection point
        t = min(t1, t2);
        return Intersection(t, r.at(t), (r.at(t) - center).normalize(), material);
    }
    */
}


/*********
 * Plane *
 *********/

// Source: https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-plane-and-ray-disk-intersection.html
optional<Intersection> Plane::intersect(const Ray& r) const {
    float denominator = normal.dot(r.direction);
    
    // If the ray is parallel to the plane, there is no intersection
    if (abs(denominator) < EPSILON)
        return nullopt;

    Point base = Point(normal.x, normal.y, normal.z) * distance; // Point of the plane
    float t = normal.dot(base - r.origin) / denominator; // Distance from the ray origin to the intersection point

    // If the intersection point is behind the ray origin, there is no intersection
    if (t < 0)
        return nullopt;

    return Intersection(t, r.at(t), normal, material);
}

float Plane::distanceTo(const Point& point) const {
    return normal.dot(point - Point(normal.x, normal.y, normal.z) * distance);
}

string Plane::toString() const {
    ostringstream oss;
    oss << "Base: " << normal*distance << "\n"
        << "Normal: " << normal;
    return oss.str();
}

/************
 * Triangle *
 ************/

// Möller-Trumbore intersection algorithm
optional<Intersection> Triangle::intersect(const Ray& r) const {
    const float EPS = 1e-8f;
    
    Direction edge1 = b - a;
    Direction edge2 = c - a;
    Direction h = r.direction.cross(edge2);
    float a_det = edge1.dot(h);

    // Check if the ray is parallel to the triangle
    if (abs(a_det) < EPS)
        return nullopt;

    float f = 1.0f / a_det;
    Direction s = r.origin - a;
    float u = f * s.dot(h);

    // Check if the intersection point is outside the triangle
    if (u < 0.0f || u > 1.0f)
        return nullopt;

    Direction q = s.cross(edge1);
    float v = f * r.direction.dot(q);

    // Check if the intersection point is outside the triangle
    if (v < 0.0f || u + v > 1.0f)
        return nullopt;

    float t = f * edge2.dot(q);

    // Check if the intersection point is in front of the ray origin
    if (t > EPS) {
        Point intersectionPoint = r.at(t);
        return Intersection(t, intersectionPoint, normal, material);
    } else {
        return nullopt;
    }
}

string Triangle::toString() const {
    ostringstream oss;
    oss << "A: " << a << "\n"
        << "B: " << b << "\n"
        << "C: " << c;
    return oss.str();
}

/********
 * Cone *
 ********/

optional<Intersection> Cone::intersect(const Ray& ray) const {
    const float EPS = 1e-8f;
    
    // Transform ray to cone's local coordinate system
    // Assume cone axis is along positive Y, with tip at (0, height, 0)
    Direction co = ray.origin - base;
    
    // Cone equation: x² + z² = (radius * (height - y) / height)²
    // Rearranging: x² + z² - (radius² * (height - y)² / height²) = 0
    
    float k = radius / height;
    k = k * k;
    
    float a = ray.direction.x * ray.direction.x + ray.direction.z * ray.direction.z - k * ray.direction.y * ray.direction.y;
    float b = 2.0f * (co.x * ray.direction.x + co.z * ray.direction.z - k * (height - co.y) * (-ray.direction.y));
    float c = co.x * co.x + co.z * co.z - k * (height - co.y) * (height - co.y);

    float discriminant = b * b - 4.0f * a * c;

    if (discriminant < 0) {
        return nullopt;
    }

    float sqrt_discriminant = sqrt(discriminant);
    float t1 = (-b - sqrt_discriminant) / (2.0f * a);
    float t2 = (-b + sqrt_discriminant) / (2.0f * a);

    // Choose the closest positive intersection
    float t = (t1 > EPS) ? t1 : t2;
    if (t <= EPS) {
        return nullopt;
    }

    Point intersectionPoint = ray.at(t);
    Direction localPoint = intersectionPoint - base;
    
    // Check if intersection is within cone bounds (0 <= y <= height)
    if (localPoint.y < 0 || localPoint.y > height) {
        return nullopt;
    }

    // Calculate normal at intersection point
    // For a cone, normal at (x, y, z) is (x, -radius²/height, z) normalized
    float normalY = -radius * radius / height;
    Direction normal = Direction(localPoint.x, normalY, localPoint.z).normalize();
    
    return Intersection(t, intersectionPoint, normal, material);
}

string Cone::toString() const {
    stringstream ss;
    ss << "Cone(base: " << base.toString() << ", axis: " << axis.toString() << ", radius: " << radius << ", height: " << height << ")";
    return ss.str();
}

/*************
 * Cylinder *
 *************/