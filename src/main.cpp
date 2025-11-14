#include <iostream>
#include <vector>

#include "IO.hpp"

int main(int argc, char *argv[])
{
    try
    {
        const std::string instancePath = "data/instances/grande_1.spp";
        const std::string configPath = "data/config/default.json";

        // 1) Leer archivo de instancia
        ProblemInstance instance = IO::readInstanceFromFile(instancePath);

        // 2) Leer p y alpha por consola
        IO::readParamsFromConsole(instance);

        // 3) Simulated Annealing
        SAConfig saCfg = IO::readConfigFromJson(configPath);
        std::vector<std::vector<int>> Z(instance.nRows, std::vector<int>(instance.nCols, 1));
        double errorTotal = 0.0;

        // 4) Escribir archivo de salida
        IO::writeSolutionToFile("data/solutions/spp.out", errorTotal, instance, Z);

        std::cout << "Archivo de salida generado.\n";
    }
    catch (const std::exception &ex)
    {
        std::cerr << "Error: " << ex.what() << '\n';
        return 1;
    }

    return 0;
}
