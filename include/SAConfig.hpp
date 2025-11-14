#pragma once

struct SAConfig
{
    double T0 = 1000.0;          // temperatura inicial
    double Tf = 0.001;           // temperatura final
    int maxIterations = 100000;  // total de iteraciones
    int itersPerTemp = 100;      // iteraciones entre cambio de temperatura
    double coolingFactor = 0.95; // alpha del enfriamiento
    double maxTimeSeconds = 5.0; // 0 = sin límite
};
