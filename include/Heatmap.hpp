#pragma once

#include <vector>
#include <string>

// Visualiza la matriz de datos (M) como mapa de calor y, opcionalmente,
// delimita las zonas indicadas por la matriz Z.
void plotHeatmap(const std::vector<std::vector<float>> &M, int factor, const std::vector<std::vector<int>> &Z = {});

// Genera la imagen de calor y la guarda en outputPath (e.g. PNG).
void saveHeatmap(const std::vector<std::vector<float>> &M, int factor, const std::vector<std::vector<int>> &Z, const std::string &outputPath);
