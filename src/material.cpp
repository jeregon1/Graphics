
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

std::optional<Direction> Material::refractar(const Direction& wo, const Direction& normal) const {
    // Proper implementation of Snell's law with entering/exiting logic
    float n1, n2;
    float cosThetaI = normal.dot(wo);
    Direction n = normal;
    
    if (cosThetaI < 0) {
        // Ray is entering the material (from air to material)
        cosThetaI = -cosThetaI;
        n1 = 1.0f; // Air
        n2 = this->n; // Material
    } else {
        // Ray is exiting the material (from material to air)
        n1 = this->n; // Material
        n2 = 1.0f; // Air
        n = -normal; // Flip normal for exiting ray
    }
    
    float eta = n1 / n2;
    float sinThetaT2 = eta * eta * (1.0f - cosThetaI * cosThetaI);
    
    if (sinThetaT2 > 1.0f)
        return std::nullopt; // Total internal reflection
    
    float cosThetaT = sqrt(1.0f - sinThetaT2);
    return std::make_optional((wo * eta + n * (eta * cosThetaI - cosThetaT)));
}

RGB Material::evaluateBSDF(const Direction& wi, const Direction& wo, const Direction& normal) const {
	(void)wi; (void)wo; (void)normal; // Suppress unused parameter warnings
	RGB result(0, 0, 0);
	
	// Lambertian diffuse component: kd/π
	result += diffuse * (1.0f / M_PI);
	
	// Perfect specular reflection and transmittance (delta functions) are handled separately
	// in path tracing through importance sampling, not direct BSDF evaluation
	
	return result;
}
