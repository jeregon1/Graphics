#pragma once

class Kernel {
    public:
    Kernel() {};
    // Cada kernel debe implementar su funcion de evaluacion
    virtual double evaluar(double radioFoton, double radioMax) const = 0;
};

class KernelCaja : public Kernel {
    public:
    double evaluar(double radioFoton, double radioMax) const;
};

class KernelTriangular : public Kernel {
    public:
    double evaluar(double radioFoton, double radioMax) const;
};

class KernelGaussiano : public Kernel {
    double sigma;
    public:
    KernelGaussiano(double s = 1) : sigma(s) {};
    double evaluar(double radioFoton, double radioMax) const;
};

class KernelEpanechnikov : public Kernel {
    public:
    double evaluar(double radioFoton, double radioMax) const;
};

class KernelQuartic : public Kernel {
    public:
    double evaluar(double radioFoton, double radioMax) const;
};

class KernelTripeso : public Kernel {
    public:
    double evaluar(double radioFoton, double radioMax) const;
};

class KernelTricubo : public Kernel {
    public:
    double evaluar(double radioFoton, double radioMax) const;
};  

class KernelCoseno : public Kernel {
    public:
    double evaluar(double radioFoton, double radioMax) const;
};

class KernelLogistico : public Kernel {
    public:
    double evaluar(double radioFoton, double radioMax) const;
};

class KernelSigmoide : public Kernel {
    public:
    double evaluar(double radioFoton, double radioMax) const;
};
