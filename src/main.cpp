#include <iostream>
#include <vector>

#include "IO.hpp"
#include "SA.hpp"

int main(int argc, char *argv[])
{
    try
    {
        const std::string instancePath = "data/instances/grande_4.spp";
        const std::string configPath = "data/config/default.json";

        // 1) Leer archivo de instancia
        ProblemInstance instance = IO::readInstanceFromFile(instancePath);

        // 2) Leer p y alpha por consola
        IO::readParamsFromConsole(instance);

        // 3) Simulated Annealing
        SAConfig saCfg = IO::readConfigFromJson(configPath);
        Solution best = simulatedAnnealing(instance, saCfg);

        // 4) Escribir archivo de salida
        IO::writeSolutionToFile("data/solutions/spp.out", best.errorTotal, instance, best.Z);

        std::cout << "Archivo de salida generado.\n";
    }
    catch (const std::exception &ex)
    {
        std::cerr << "Error: " << ex.what() << '\n';
        return 1;
    }

    return 0;
}
