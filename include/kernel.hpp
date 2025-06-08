#pragma

#include <cmath>

// TODO: Poner solo los kernels necesarios
class Kernel {
    public:
    Kernel() {};

    double evaluarCaja(double radioFoton, double radioMax) {
        return 1 / (M_PI * radioMax * radioMax);
    }

    double evaluarTriangular(double radioFoton, double radioMax) {
        return (1 - radioFoton / radioMax) / (M_PI * radioMax * radioMax);
    }

    double evaluarGaussiano(double radioFoton, double radioMax, double sigma) {
        return exp(-radioFoton * radioFoton / (2 * sigma * sigma)) /
            (sqrt(2 * M_PI) * sigma * M_PI * radioMax * radioMax);
    }

    double evaluarEpanechnikov(double radioFoton, double radioMax) {
        return 3.0 / (4.0 * M_PI * radioMax * radioMax) *
            (1 - radioFoton * radioFoton / (radioMax * radioMax));
    }

    double evaluarQuartic(double radioFoton, double radioMax) {
        return 15.0 / (16.0 * M_PI * radioMax * radioMax) *
            pow(1 - radioFoton * radioFoton / (radioMax * radioMax), 2);
    }

    double evaluarTripeso(double radioFoton, double radioMax) {
        return 35.0 / (32.0 * M_PI * radioMax * radioMax) *
            pow(1 - radioFoton * radioFoton / (radioMax * radioMax), 3);
    }

    double evaluarTricubo(double radioFoton, double radioMax) {
        return 70.0 / (81.0 * M_PI * radioMax * radioMax) *
            pow(1 - pow(radioFoton / radioMax, 3), 3);
    }

    double evaluarCoseno(double radioFoton, double radioMax) {
        return 1.0 / (4.0 * radioMax * radioMax) * cos(M_PI * radioFoton / radioMax / 2.0);
    }

    double evaluarLogistico(double radioFoton, double radioMax) {
        return 1.0 / (2.0 * radioMax * radioMax) /
            (exp(radioFoton / radioMax) + 2.0 + exp(-radioFoton / radioMax));
    }

};
