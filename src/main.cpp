#include <iostream>
#include <string>
#include <vector>

#include "IO.hpp"
#include "SA.hpp"

int main(int argc, char *argv[])
{
    try
    {
        const std::string instancePath = "data/instances/grande_5.spp";
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

        std::cout << "Archivos de salida generados:\n"
                  << " - " << initialPath << '\n'
                  << " - " << bestPath << '\n';
    }
    catch (const std::exception &ex)
    {
        std::cerr << "Error: " << ex.what() << '\n';
        return 1;
    }

    return 0;
}
