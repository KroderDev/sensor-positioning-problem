#include <algorithm>
#include <filesystem>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include "IO.hpp"
#include "SA.hpp"
#include "Heatmap.hpp"

namespace
{
    std::vector<std::vector<float>> toFloatMatrix(const std::vector<std::vector<double>> &matrix)
    {
        std::vector<std::vector<float>> result;
        result.reserve(matrix.size());

        for (const auto &row : matrix)
        {
            std::vector<float> floatRow;
            floatRow.reserve(row.size());
            for (double value : row)
            {
                floatRow.push_back(static_cast<float>(value));
            }
            result.push_back(std::move(floatRow));
        }

        return result;
    }

    int chooseScaleFactor(int rows, int cols)
    {
        const int maxDim = std::max(rows, cols);
        if (maxDim >= 50)
        {
            return 10;
        }
        if (maxDim >= 30)
        {
            return 15;
        }
        if (maxDim >= 15)
        {
            return 20;
        }
        return 25;
    }
}

int main(int argc, char *argv[])
{
    try
    {
        const std::string instancePath = "data/instances/instance.spp";
        const std::string configPath = "data/config/default.json";

        // Obtener nombre de la instancia.
        std::string instanceName = instancePath;
        auto slashPos = instanceName.find_last_of("/\\");
        if (slashPos != std::string::npos)
        {
            instanceName = instanceName.substr(slashPos + 1);
        }
        auto dotPos = instanceName.find_last_of('.');
        if (dotPos != std::string::npos)
        {
            instanceName = instanceName.substr(0, dotPos);
        }

        // 1) Leer archivo de instancia
        ProblemInstance instance = IO::readInstanceFromFile(instancePath);

        // 2) Leer p y alpha por consola
        IO::readParamsFromConsole(instance);

        // 3) Simulated Annealing
        SAConfig saCfg = IO::readConfigFromJson(configPath);
        Solution initial;
        Solution best = simulatedAnnealing(instance, saCfg, &initial);

        // 4) Escribir archivos de salida (antes y después de SA)
        std::string initialPath = "data/solutions/" + instanceName + "_initial.out";
        std::string bestPath = "data/solutions/" + instanceName + "_best.out";
        IO::writeSolutionToFile(initialPath, initial.errorTotal, instance, initial.Z);
        IO::writeSolutionToFile(bestPath, best.errorTotal, instance, best.Z);

        std::string heatmapPath;
        if (!best.Z.empty())
        {
            const auto dataAsFloat = toFloatMatrix(instance.S);
            const int scaleFactor = chooseScaleFactor(instance.nRows, instance.nCols);
            heatmapPath = bestPath + ".png";
            std::filesystem::create_directories(std::filesystem::path(heatmapPath).parent_path());
            saveHeatmap(dataAsFloat, scaleFactor, best.Z, heatmapPath);
        }

        std::cout << "Archivos de salida generados:\n"
                  << " - " << initialPath << '\n'
                  << " - " << bestPath << '\n';
        if (!heatmapPath.empty())
        {
            std::cout << " - " << heatmapPath << '\n';
        }
    }
    catch (const std::exception &ex)
    {
        std::cerr << "Error: " << ex.what() << '\n';
        return 1;
    }

    return 0;
}
