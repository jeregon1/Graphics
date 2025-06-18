#pragma once

#include <string>
#include <iostream>

class Kernel {
    public:
    Kernel() {};
    // Cada kernel debe implementar su funcion de evaluacion
    virtual double evaluar(double radioFoton, double radioMax) const = 0;
    virtual std::string toString() const = 0;
    friend std::ostream& operator<<(std::ostream& os, const Kernel& kernel) {
        return os << kernel.toString();
    }
};

class KernelCaja : public Kernel {
    public:
    double evaluar(double radioFoton, double radioMax) const;
    std::string toString() const { return "KernelCaja"; }
};

class KernelTriangular : public Kernel {
    public:
    double evaluar(double radioFoton, double radioMax) const;
    std::string toString() const { return "KernelTriangular"; }
};

class KernelGaussiano : public Kernel {
    double sigma;
    public:
    KernelGaussiano(double s = 1) : sigma(s) {};
    double evaluar(double radioFoton, double radioMax) const;
    std::string toString() const { return "KernelGaussiano"; }
};

class KernelEpanechnikov : public Kernel {
    public:
    double evaluar(double radioFoton, double radioMax) const;
    std::string toString() const { return "KernelEpanechnikov"; }
};

class KernelQuartic : public Kernel {
    public:
    double evaluar(double radioFoton, double radioMax) const;
    std::string toString() const { return "KernelQuartic"; }
};

class KernelTripeso : public Kernel {
    public:
    double evaluar(double radioFoton, double radioMax) const;
    std::string toString() const { return "KernelTripeso"; }
};

class KernelTricubo : public Kernel {
    public:
    double evaluar(double radioFoton, double radioMax) const;
    std::string toString() const { return "KernelTricubo"; }
};  

class KernelCoseno : public Kernel {
    public:
    double evaluar(double radioFoton, double radioMax) const;
    std::string toString() const { return "KernelCoseno"; }
};

class KernelLogistico : public Kernel {
    public:
    double evaluar(double radioFoton, double radioMax) const;
    std::string toString() const { return "KernelLogistico"; }
};

class KernelSigmoide : public Kernel {
    public:
    double evaluar(double radioFoton, double radioMax) const;
    std::string toString() const { return "KernelSigmoide"; }
};
