#pragma once

#include <vector>

struct ProblemInstance
{
    int nRows = 0;                      // N
    int nCols = 0;                      // M
    std::vector<std::vector<double>> S; // matriz S[N][M]

    int p = 0;        // nº de zonas/sensores
    double alpha = 0; // nivel de homogeneidad
};