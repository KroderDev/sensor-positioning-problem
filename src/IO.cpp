#include "IO.hpp"
#include <fstream>
#include <iostream>
#include <limits>
#include <stdexcept>
#include "third_party/json.hpp"

using nlohmann::json;
using std::cin;
using std::cout;
using std::endl;

namespace IO
{
    ProblemInstance readInstanceFromFile(const std::string &path)
    {
        std::ifstream in(path);
        if (!in)
        {
            throw std::runtime_error("No se pudo abrir el archivo de instancia: " + path);
        }

        ProblemInstance inst;

        // N y M
        if (!(in >> inst.nRows >> inst.nCols))
        {
            throw std::runtime_error("Error al leer N y M desde el archivo: " + path);
        }

        if (inst.nRows <= 0 || inst.nCols <= 0)
        {
            throw std::runtime_error("N y M deben ser positivos en el archivo: " + path);
        }

        inst.S.assign(inst.nRows, std::vector<double>(inst.nCols, 0.0));

        for (int i = 0; i < inst.nRows; ++i)
        {
            for (int j = 0; j < inst.nCols; ++j)
            {
                if (!(in >> inst.S[i][j]))
                {
                    throw std::runtime_error("No se pudieron leer todos los valores de S en el archivo: " + path);
                }
            }
        }

        return inst;
    }

    void readParamsFromConsole(ProblemInstance &instance)
    {

        // p: cantidad de sensores/zonas
        while (true)
        {
            cout << "Ingrese la cantidad de sensores/zonas (p > 0): ";
            if (cin >> instance.p && instance.p > 0)
            {
                break;
            }
            cout << "Valor invalido. p debe ser un entero positivo." << endl;
            cin.clear();
            cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }

        // alpha: nivel de homogeneidad
        while (true)
        {
            cout << "Ingrese el nivel de homogeneidad (alpha > 0): ";
            if (cin >> instance.alpha && instance.alpha > 0.0)
            {
                break;
            }
            cout << "Valor invalido. alpha debe ser un numero positivo." << endl;
            cin.clear();
            cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }
    }

    SAConfig readConfigFromJson(const std::string &path)
    {
        std::ifstream in(path);
        if (!in)
        {
            throw std::runtime_error("No se pudo abrir el archivo de config: " + path);
        }

        json j;
        in >> j;

        SAConfig cfg;

        // Usamos value(clave, por_defecto) por si falta algo en el JSON
        cfg.T0 = j.value("T0", 1000.0);
        cfg.Tf = j.value("Tf", 0.001);
        cfg.maxIterations = j.value("max_iterations", 100000);
        cfg.itersPerTemp = j.value("iters_per_temp", 100);
        cfg.coolingFactor = j.value("cooling_factor", 0.95);
        cfg.maxTimeSeconds = j.value("max_time_seconds", 5.0);

        return cfg;
    }

    void writeSolutionToFile(const std::string &path, double errorTotal, const ProblemInstance &instance, const std::vector<std::vector<int>> &Z)
    {
        if (static_cast<int>(Z.size()) != instance.nRows)
        {
            throw std::runtime_error("La matriz Z no coincide con la cantidad de filas del problema.");
        }
        for (const auto &row : Z)
        {
            if (static_cast<int>(row.size()) != instance.nCols)
            {
                throw std::runtime_error("La matriz Z no coincide con la cantidad de columnas del problema.");
            }
        }

        std::ofstream out(path);
        if (!out)
        {
            throw std::runtime_error("No se pudo abrir el archivo de salida: " + path);
        }

        out << errorTotal << '\n'
            << '\n';

        for (int i = 0; i < instance.nRows; ++i)
        {
            for (int j = 0; j < instance.nCols; ++j)
            {
                out << Z[i][j];
                if (j + 1 < instance.nCols)
                {
                    out << ' ';
                }
            }
            out << '\n';
        }
    }
}
