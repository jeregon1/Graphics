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

MapaFotones Scene::generarMapaFotones(int nPaths, bool save, double sigma) const {
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

// TODO: Refactorizar nombres de variables y funciones
RGB Scene::ecuacionRenderFotones(Point point, Direction wo, Material material, Direction normal, 
    MapaFotones mapa, int kFotones, double radio, bool guardar, Kernel* kernel, double sigma) const {
    
    // Caso base
    if (material.isEmissive) {
        return material.diffuse;
    } 

    double radioFotonMasLejano = 0.0;
    double radioFoton = 0.0;
    Point posFoton;
    RGB L = RGB(0, 0, 0);

    double probability = (double) rand0_1(); // Probabilidad aleatoria entre 0 y 1

    // Seguimos hasta llegar a una superficie difusa, simulando el camino del foton
    while (probability <= material.p_diffuse + material.p_specular + material.p_transparency) {

        if (probability <= material.p_diffuse) {
            if (wo * normal > 0.0) {
                normal = Direction(-normal.x, -normal.y, -normal.z); // Dirección del rayo * normal de la intersección
            } 
            wo = wo - normal * 2.0f * (wo * normal); // Ecuación de reflexión
        } else { // Especular
            wo = material.refractar(wo, normal); // Ecuación de refracción
        }

        // Se maneja siguiente intersección
        auto intersection = this->intersect(Ray(point, wo));
        if (!intersection) {
            return L; // Si no hay intersección, se devuelve la luz acumulada
        } else {
            point = intersection->point;
            material = intersection->material;
            normal = intersection->normal;
            probability = (double) rand0_1();
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
                L = L + (material.diffuse / material.p_diffuse) * f->flujo
                    *kernel->evaluar((posFoton - point).mod(), radioFotonMasLejano);
            }
        }
        // Estimacion de la luz directa
        if (!guardar) L = L + estimacionSiguienteEvento(point, wo, material, normal, sigma);
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
                if (sigma == 0.0) L = L + (fr * coseno) * (lights[i]->light / norma);
                else L = L + (fr * coseno) * (lights[i]->light / norma) * pow(M_E, -sigma * norma);
            }
        }
    }
    return L;
}

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

    // If the intersection point is behind the ray origin, it means there's no intersection
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

optional<Intersection> Cylinder::intersect(const Ray& ray) const {
    const float EPS = 1e-8f;
    
    // Transform ray to cylinder's local coordinate system
    // Assume cylinder axis is along Y direction
    Direction co = ray.origin - base;
    
    // For a cylinder with axis along Y, equation is: x² + z² = radius²
    // We only consider the x and z components for the cylindrical surface
    
    float a = ray.direction.x * ray.direction.x + ray.direction.z * ray.direction.z;
    
    // If ray is parallel to cylinder axis, no intersection with curved surface
    if (abs(a) < EPS) {
        return nullopt;
    }
    
    float b = 2.0f * (co.x * ray.direction.x + co.z * ray.direction.z);
    float c = co.x * co.x + co.z * co.z - radius * radius;

    float discriminant = b * b - 4.0f * a * c;

    if (discriminant < 0) {
        return nullopt;
    }

    float sqrt_discriminant = sqrt(discriminant);
    float t1 = (-b - sqrt_discriminant) / (2.0f * a);
    float t2 = (-b + sqrt_discriminant) / (2.0f * a);

    // Check both intersections and choose the closest valid one
    float t = -1;
    
    // Check first intersection
    if (t1 > EPS) {
        Point testPoint = ray.at(t1);
        Direction localPoint = testPoint - base;
        if (localPoint.y >= 0 && localPoint.y <= height) {
            t = t1;
        }
    }
    
    // Check second intersection if first wasn't valid
    if (t < 0 && t2 > EPS) {
        Point testPoint = ray.at(t2);
        Direction localPoint = testPoint - base;
        if (localPoint.y >= 0 && localPoint.y <= height) {
            t = t2;
        }
    }
    
    if (t <= EPS) {
        return nullopt;
    }

    Point intersectionPoint = ray.at(t);
    Direction localPoint = intersectionPoint - base;
    
    // Calculate normal at intersection point
    // For a cylinder, normal is perpendicular to the axis and pointing outward
    Direction normal = Direction(localPoint.x, 0, localPoint.z).normalize();
    
    return Intersection(t, intersectionPoint, normal, material);
}

string Cylinder::toString() const {
    stringstream ss;
    ss << "Cylinder(base: " << base.toString() << ", axis: " << axis.toString() 
       << ", radius: " << radius << ", height: " << height << ")";
    return ss.str();
}