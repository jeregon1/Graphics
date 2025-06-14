#pragma once

#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

class Kernel {
    public:
    Kernel() {};

    double evaluateBox(double photonRadius, double maxRadius) {
        return 1.0 / (M_PI * maxRadius * maxRadius);
    }

    double evaluateTriangular(double photonRadius, double maxRadius) {
        return (1 - photonRadius / maxRadius) / (M_PI * maxRadius * maxRadius);
    }

    double evaluateGaussian(double photonRadius, double maxRadius, double sigma) {
        return exp(-photonRadius * photonRadius / (2 * sigma * sigma)) /
            (sqrt(2 * M_PI) * sigma * M_PI * maxRadius * maxRadius);
    }

    double evaluateEpanechnikov(double photonRadius, double maxRadius) {
        return 3.0 / (4.0 * M_PI * maxRadius * maxRadius) *
            (1 - photonRadius * photonRadius / (maxRadius * maxRadius));
    }

    double evaluateQuartic(double photonRadius, double maxRadius) {
        return 15.0 / (16.0 * M_PI * maxRadius * maxRadius) *
            pow(1 - photonRadius * photonRadius / (maxRadius * maxRadius), 2);
    }

    double evaluateTriweight(double photonRadius, double maxRadius) {
        return 35.0 / (32.0 * M_PI * maxRadius * maxRadius) *
            pow(1 - photonRadius * photonRadius / (maxRadius * maxRadius), 3);
    }

    double evaluateTricube(double photonRadius, double maxRadius) {
        return 70.0 / (81.0 * M_PI * maxRadius * maxRadius) *
            pow(1 - pow(photonRadius / maxRadius, 3), 3);
    }

    double evaluateCosine(double photonRadius, double maxRadius) {
        return 1.0 / (4.0 * maxRadius * maxRadius) * cos(M_PI * photonRadius / maxRadius / 2.0);
    }

    double evaluateLogistic(double photonRadius, double maxRadius) {
        return 1.0 / (2.0 * maxRadius * maxRadius) /
            (exp(photonRadius / maxRadius) + 2.0 + exp(-photonRadius / maxRadius));
    }

    // General evaluate method (defaults to box kernel)
    double evaluate(double photonRadius, double maxRadius) {
        return evaluateBox(photonRadius, maxRadius);
    }
};
