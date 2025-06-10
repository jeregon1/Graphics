#pragma once

class Kernel {
    public:
    Kernel() {};
    // Cada kernel debe implementar su funcion de evaluacion
    virtual double evaluar(double radioFoton, double radioMax) = 0;
};

class KernelCaja : public Kernel {
    public:
    double evaluar(double radioFoton, double radioMax);
};

class KernelTriangular : public Kernel {
    public:
    double evaluar(double radioFoton, double radioMax);
};

class KernelGaussiano : public Kernel {
    double sigma;
    public:
    KernelGaussiano(double s = 1) : sigma(s) {};
    double evaluar(double radioFoton, double radioMax);
};

class KernelEpanechnikov : public Kernel {
    public:
    double evaluar(double radioFoton, double radioMax);
};

class KernelQuartic : public Kernel {
    public:
    double evaluar(double radioFoton, double radioMax);
};

class KernelTripeso : public Kernel {
    public:
    double evaluar(double radioFoton, double radioMax);
};

class KernelTricubo : public Kernel {
    public:
    double evaluar(double radioFoton, double radioMax);
};  

class KernelCoseno : public Kernel {
    public:
    double evaluar(double radioFoton, double radioMax);
};

class KernelLogistico : public Kernel {
    public:
    double evaluar(double radioFoton, double radioMax);
};

class KernelSigmoide : public Kernel {
    public:
    double evaluar(double radioFoton, double radioMax);
};
