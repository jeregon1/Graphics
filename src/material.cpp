#include <algorithm>
#include "material.hpp"

void Material::normalize() {
	for (int i = 0; i < 3; ++i) {
		float& d = (i == 0) ? diffuse.r : (i == 1) ? diffuse.g : diffuse.b;
		float& s = (i == 0) ? specular.r : (i == 1) ? specular.g : specular.b;
		float& t = (i == 0) ? transmittance.r : (i == 1) ? transmittance.g : transmittance.b;
		
		float total = d + s + t;
		if (total >= 1.0f) {
			float scale = 0.99f / total;
			d *= scale;
			s *= scale;
			t *= scale;
		}
	}
	
	// Update probabilities after normalization
	p_diffuse = diffuse.max();
	p_specular = specular.max();
	p_transmittance = transmittance.max();

	double totalProbability = p_diffuse + p_specular + p_transmittance;
	if (totalProbability > 0.0) {
		p_diffuse      *= p_limit / totalProbability;
		p_specular     *= p_limit / totalProbability;
		p_transmittance *= p_limit / totalProbability;
	}
}

// Replace existing Material::refract with robust implementation using Snell's law
std::optional<Direction> Material::refract(const Direction& wi, const Direction& N) const {
    float cosi = std::max(-1.0f, std::min(1.0f, wi.dot(N))); // Clamp to [-1, 1] to prevent floating point errors
    float etai = 1.0f, etat = n;
    Direction normal = N;
    
    if (cosi < 0) {
        // Ray entering material (air -> material)
        cosi = -cosi;
    } else {
        // Ray exiting material (material -> air)  
        std::swap(etai, etat);
        normal = -N;
        // cosi is already positive
    }
    
    float eta = etai / etat;
    float k = 1.0f - eta*eta * (1.0f - cosi*cosi);
    
    if (k < 0.0f)
        return std::nullopt; // Total internal reflection
    
    Direction refr = wi * eta + normal * (eta * cosi - std::sqrt(k)); // refr = θ₁ * n + N * (θ₁ * cosi - sqrt(cos²(θ) )
    return std::make_optional(refr.normalize());
}

RGB Material::evaluateBSDF(const Direction& wi, const Direction& wo, const Direction& normal) const {
	(void)wi; (void)wo; (void)normal; // Suppress unused parameter warnings
	RGB result(0, 0, 0);
	
	// Lambertian diffuse component: kd/π
	result += diffuse / M_PI;
	
	// Perfect specular reflection and transmittance (delta functions) are handled separately
	// in path tracing through importance sampling, not direct BSDF evaluation
	
	return result;
}
