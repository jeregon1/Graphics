#include "kernel.hpp"
#include <cmath>

// Box kernel (uniform weight within radius)
double KernelCaja::evaluar(double radioFoton, double radioMax) const {
    if (radioFoton <= radioMax) {
        return 1.0;
    }
    return 0.0;
}

// Triangular kernel (linear falloff)
double KernelTriangular::evaluar(double radioFoton, double radioMax) const {
    if (radioFoton <= radioMax && radioMax > 0.0) {
        return 1.0 - (radioFoton / radioMax);
    }
    return 0.0;
}

// Gaussian kernel
double KernelGaussiano::evaluar(double radioFoton, double radioMax) const {
    if (radioMax <= 0.0) return 0.0;
    double t = radioFoton / radioMax;
    return exp(-0.5 * t * t / (sigma * sigma));
}

// Epanechnikov kernel
double KernelEpanechnikov::evaluar(double radioFoton, double radioMax) const {
    if (radioFoton <= radioMax && radioMax > 0.0) {
        double t = radioFoton / radioMax;
        return 1.0 - t * t;
    }
    return 0.0;
}

// Quartic kernel
double KernelQuartic::evaluar(double radioFoton, double radioMax) const {
    if (radioFoton <= radioMax && radioMax > 0.0) {
        double t = radioFoton / radioMax;
        double t2 = t * t;
        return (1.0 - t2) * (1.0 - t2);
    }
    return 0.0;
}

// Tripeso kernel
double KernelTripeso::evaluar(double radioFoton, double radioMax) const {
    if (radioFoton <= radioMax && radioMax > 0.0) {
        double t = radioFoton / radioMax;
        return (1.0 - t * t * t) * (1.0 - t * t * t) * (1.0 - t * t * t);
    }
    return 0.0;
}

// Tricubo kernel
double KernelTricubo::evaluar(double radioFoton, double radioMax) const {
    if (radioFoton <= radioMax && radioMax > 0.0) {
        double t = radioFoton / radioMax;
        double factor = 1.0 - t * t * t;
        return factor * factor * factor;
    }
    return 0.0;
}

// Coseno kernel
double KernelCoseno::evaluar(double radioFoton, double radioMax) const {
    if (radioFoton <= radioMax && radioMax > 0.0) {
        double t = radioFoton / radioMax;
        return cos(M_PI * t / 2.0);
    }
    return 0.0;
}

// Logistico kernel
double KernelLogistico::evaluar(double radioFoton, double radioMax) const {
    if (radioMax <= 0.0) return 0.0;
    double t = radioFoton / radioMax;
    return 1.0 / (exp(t) + 2.0 + exp(-t));
}

// Sigmoide kernel
double KernelSigmoide::evaluar(double radioFoton, double radioMax) const {
    if (radioMax <= 0.0) return 0.0;
    double t = radioFoton / radioMax;
    return 1.0 / (exp(t - 1.0) + exp(1.0 - t));
}
