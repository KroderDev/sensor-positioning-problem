#pragma once

#include <random>
#include <vector>
#include "ProblemInstance.hpp"

// Representa una solución del SPP:
//  - Z: matriz de etiquetas de zonas (1..p)
//  - errorTotal: valor de la función objetivo (suma de errores cuadrados)
struct Solution
{
    std::vector<std::vector<int>> Z;
    double errorTotal = 0.0;
};

// Crea una solución inicial dividiendo las filas en franjas consecutivas.
Solution buildInitialSolution(const ProblemInstance &instance);

// Calcula medias, varianzas y error total de la asignación Z.
double calculateErrorAndVariance(const ProblemInstance &instance, const std::vector<std::vector<int>> &Z, std::vector<double> &means, std::vector<double> &variances, std::vector<int> &counts);

// Penaliza el exceso de varianza respecto de alpha * Var(S) para cada zona.
double calculateVariancePenalty(const ProblemInstance &instance, const std::vector<double> &variances, const std::vector<int> &counts, double totalVariance);

// Calcula la varianza total de todos los S[i][j].
double calculateTotalVariance(const ProblemInstance &instance);

// Verifica homogeneidad y conexidad de todas las zonas.
bool isSolutionValid(const ProblemInstance &instance, const std::vector<std::vector<int>> &Z, double totalVariance);

// Verifica solo conexidad (polígono válido) y que no existan zonas vacías.
bool isPartitionConnected(const ProblemInstance &instance, const std::vector<std::vector<int>> &Z);

// Intenta reparar zonas a rectángulos no superpuestos. Devuelve true si pudo.
bool makeRectsIfNonOverlapping(const ProblemInstance &instance, std::vector<std::vector<int>> &Z);

// Genera un vecino moviendo un borde completo cuando es posible.
// Devuelve true si se generó un vecino distinto de la solución actual.
bool generateNeighbor(const ProblemInstance &instance, const std::vector<std::vector<int>> &currentZ, std::vector<std::vector<int>> &neighborZ, std::mt19937 &rng);
