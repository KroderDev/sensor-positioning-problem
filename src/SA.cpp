#include "SA.hpp"

#include <chrono>
#include <cmath>
#include <random>

Solution simulatedAnnealing(const ProblemInstance &instance, const SAConfig &cfg, Solution *initialOut)
{
    Solution current = buildInitialSolution(instance);
    double totalVariance = calculateTotalVariance(instance);

    std::mt19937 rng(std::random_device{}());

    // Intentar encontrar una solución inicial válida si la partición por franjas no cumple restricciones
    if (!isSolutionValid(instance, current.Z, totalVariance))
    {
        std::vector<std::vector<int>> neighborZ;
        for (int attempt = 0; attempt < 1000; ++attempt)
        {
            if (!generateNeighbor(instance, current.Z, neighborZ, rng))
            {
                continue;
            }
            if (isSolutionValid(instance, neighborZ, totalVariance))
            {
                std::vector<double> means(instance.p + 1, 0.0), variances(instance.p + 1, 0.0);
                std::vector<int> counts(instance.p + 1, 0);
                current.errorTotal = calculateErrorAndVariance(instance, neighborZ, means, variances, counts);
                current.Z = neighborZ;
                break;
            }
        }
    }

    if (initialOut)
    {
        *initialOut = current;
    }

    Solution best = current;

    double temperature = cfg.T0;
    int iterations = 0;

    std::uniform_real_distribution<double> uniform01(0.0, 1.0);

    const auto startTime = std::chrono::steady_clock::now();

    auto timeExceeded = [&]() {
        if (cfg.maxTimeSeconds <= 0.0)
        {
            return false;
        }
        auto elapsed = std::chrono::duration<double>(std::chrono::steady_clock::now() - startTime).count();
        return elapsed >= cfg.maxTimeSeconds;
    };

    while (temperature > cfg.Tf && iterations < cfg.maxIterations && !timeExceeded())
    {
        for (int i = 0; i < cfg.itersPerTemp && iterations < cfg.maxIterations && !timeExceeded(); ++i)
        {
            ++iterations;
            std::vector<std::vector<int>> neighborZ;
            if (!generateNeighbor(instance, current.Z, neighborZ, rng))
            {
                continue;
            }

            if (!isSolutionValid(instance, neighborZ, totalVariance))
            {
                continue;
            }

            std::vector<double> means(instance.p + 1, 0.0), variances(instance.p + 1, 0.0);
            std::vector<int> counts(instance.p + 1, 0);
            double neighborError = calculateErrorAndVariance(instance, neighborZ, means, variances, counts);
            double delta = neighborError - current.errorTotal;

            bool accept = false;
            if (delta < 0)
            {
                accept = true;
            }
            else
            {
                double prob = std::exp(-delta / temperature);
                accept = uniform01(rng) < prob;
            }

            if (accept)
            {
                current.Z = std::move(neighborZ);
                current.errorTotal = neighborError;
            }

            if (current.errorTotal < best.errorTotal)
            {
                best = current;
            }
        }

        temperature *= cfg.coolingFactor;
    }

    return best;
}
