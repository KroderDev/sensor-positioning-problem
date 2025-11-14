#pragma once

#include <string>
#include "ProblemInstance.hpp"
#include "SAConfig.hpp"

namespace IO
{

    // Lee el archivo de instancia (N, M y matriz S).
    ProblemInstance readInstanceFromFile(const std::string &path);

    // Lee p y alpha desde la consola.
    // Valida que p > 0 y alpha > 0.
    void readParamsFromConsole(ProblemInstance &instance);

    // Lee el archivo de configuración
    SAConfig readConfigFromJson(const std::string& path);

    // Escribe el archivo de salida con:
    //  - primera línea: errorTotal
    //  - línea en blanco
    //  - matriz de etiquetas Z de tamaño N x M
    void writeSolutionToFile(const std::string &path, double errorTotal, const ProblemInstance &instance, const std::vector<std::vector<int>> &Z);
}