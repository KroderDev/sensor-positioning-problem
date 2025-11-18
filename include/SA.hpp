#pragma once

#include "ProblemInstance.hpp"
#include "SAConfig.hpp"
#include "Solution.hpp"

// Ejecuta el algoritmo de Simulated Annealing y devuelve la mejor solución encontrada.
Solution simulatedAnnealing(const ProblemInstance &instance, const SAConfig &cfg);
