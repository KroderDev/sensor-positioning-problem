#pragma once

#include "ProblemInstance.hpp"
#include "SAConfig.hpp"
#include "Solution.hpp"

// Ejecuta el algoritmo de Simulated Annealing y devuelve la mejor solución encontrada.
// Si initialOut != nullptr, también devuelve la solución inicial antes de SA.
Solution simulatedAnnealing(const ProblemInstance &instance, const SAConfig &cfg, Solution *initialOut = nullptr);
