#include "Solution.hpp"

#include <algorithm>
#include <cmath>
#include <queue>
#include <random>

namespace
{
    using Grid = std::vector<std::vector<int>>;

    bool isZoneConnected(const Grid &Z, int zoneId)
    {
        const int nRows = static_cast<int>(Z.size());
        const int nCols = static_cast<int>(Z.empty() ? 0 : Z.front().size());

        int startRow = -1, startCol = -1;
        int totalCells = 0;
        for (int i = 0; i < nRows; ++i)
        {
            for (int j = 0; j < nCols; ++j)
            {
                if (Z[i][j] == zoneId)
                {
                    ++totalCells;
                    if (startRow == -1)
                    {
                        startRow = i;
                        startCol = j;
                    }
                }
            }
        }

        if (totalCells == 0)
        {
            return false;
        }

        std::queue<std::pair<int, int>> q;
        std::vector<std::vector<bool>> visited(nRows, std::vector<bool>(nCols, false));
        q.emplace(startRow, startCol);
        visited[startRow][startCol] = true;
        int visitedCount = 0;

        const int dr[4] = {1, -1, 0, 0};
        const int dc[4] = {0, 0, 1, -1};

        while (!q.empty())
        {
            auto [r, c] = q.front();
            q.pop();
            ++visitedCount;

            for (int k = 0; k < 4; ++k)
            {
                int nr = r + dr[k];
                int nc = c + dc[k];
                if (nr < 0 || nr >= nRows || nc < 0 || nc >= nCols)
                {
                    continue;
                }
                if (visited[nr][nc] || Z[nr][nc] != zoneId)
                {
                    continue;
                }
                visited[nr][nc] = true;
                q.emplace(nr, nc);
            }
        }

        return visitedCount == totalCells;
    }
}

Solution buildInitialSolution(const ProblemInstance &instance)
{
    Solution sol;
    sol.Z.assign(instance.nRows, std::vector<int>(instance.nCols, 1));

    int rowsPerZone = std::max(1, instance.nRows / instance.p);
    int currentZone = 1;
    int row = 0;
    while (row < instance.nRows)
    {
        for (int i = 0; i < rowsPerZone && row < instance.nRows; ++i, ++row)
        {
            for (int j = 0; j < instance.nCols; ++j)
            {
                sol.Z[row][j] = currentZone;
            }
        }
        if (currentZone < instance.p)
        {
            ++currentZone;
        }
    }

    std::vector<double> means(instance.p + 1, 0.0), variances(instance.p + 1, 0.0);
    std::vector<int> counts(instance.p + 1, 0);
    sol.errorTotal = calculateErrorAndVariance(instance, sol.Z, means, variances, counts);
    return sol;
}

double calculateErrorAndVariance(const ProblemInstance &instance,
                                 const std::vector<std::vector<int>> &Z,
                                 std::vector<double> &means,
                                 std::vector<double> &variances,
                                 std::vector<int> &counts)
{
    const int p = instance.p;
    means.assign(p + 1, 0.0);
    variances.assign(p + 1, 0.0);
    counts.assign(p + 1, 0);

    for (int i = 0; i < instance.nRows; ++i)
    {
        for (int j = 0; j < instance.nCols; ++j)
        {
            int k = Z[i][j];
            means[k] += instance.S[i][j];
            counts[k] += 1;
        }
    }

    for (int k = 1; k <= p; ++k)
    {
        if (counts[k] > 0)
        {
            means[k] /= static_cast<double>(counts[k]);
        }
    }

    double errorTotal = 0.0;

    for (int i = 0; i < instance.nRows; ++i)
    {
        for (int j = 0; j < instance.nCols; ++j)
        {
            int k = Z[i][j];
            double diff = instance.S[i][j] - means[k];
            variances[k] += diff * diff;
            errorTotal += diff * diff;
        }
    }

    for (int k = 1; k <= p; ++k)
    {
        if (counts[k] > 0)
        {
            variances[k] /= static_cast<double>(counts[k]);
        }
    }

    return errorTotal;
}

double calculateTotalVariance(const ProblemInstance &instance)
{
    double sum = 0.0;
    double sumSq = 0.0;
    int total = instance.nRows * instance.nCols;

    for (const auto &row : instance.S)
    {
        for (double val : row)
        {
            sum += val;
            sumSq += val * val;
        }
    }

    double mean = sum / static_cast<double>(total);
    double variance = (sumSq / static_cast<double>(total)) - mean * mean;
    return variance;
}

bool isSolutionValid(const ProblemInstance &instance,
                     const std::vector<std::vector<int>> &Z,
                     double totalVariance)
{
    std::vector<double> means(instance.p + 1, 0.0), variances(instance.p + 1, 0.0);
    std::vector<int> counts(instance.p + 1, 0);
    calculateErrorAndVariance(instance, Z, means, variances, counts);

    for (int k = 1; k <= instance.p; ++k)
    {
        if (counts[k] == 0)
        {
            return false;
        }
        if (variances[k] > instance.alpha * totalVariance)
        {
            return false;
        }
        if (!isZoneConnected(Z, k))
        {
            return false;
        }
    }
    return true;
}

bool generateNeighbor(const ProblemInstance &instance,
                      const std::vector<std::vector<int>> &currentZ,
                      std::vector<std::vector<int>> &neighborZ,
                      std::mt19937 &rng)
{
    const int nRows = instance.nRows;
    const int nCols = instance.nCols;
    neighborZ = currentZ;

    std::uniform_int_distribution<int> rowDist(0, nRows - 1);
    std::uniform_int_distribution<int> colDist(0, nCols - 1);

    for (int attempt = 0; attempt < 100; ++attempt)
    {
        int r = rowDist(rng);
        int c = colDist(rng);
        int currentZone = currentZ[r][c];

        std::vector<int> candidates;
        const int dr[4] = {1, -1, 0, 0};
        const int dc[4] = {0, 0, 1, -1};
        for (int k = 0; k < 4; ++k)
        {
            int nr = r + dr[k];
            int nc = c + dc[k];
            if (nr < 0 || nr >= nRows || nc < 0 || nc >= nCols)
            {
                continue;
            }
            int neighborZone = currentZ[nr][nc];
            if (neighborZone != currentZone)
            {
                candidates.push_back(neighborZone);
            }
        }

        if (candidates.empty())
        {
            continue;
        }

        std::uniform_int_distribution<size_t> pickDist(0, candidates.size() - 1);
        int newZone = candidates[pickDist(rng)];
        if (newZone == currentZone)
        {
            continue;
        }

        neighborZ[r][c] = newZone;
        return true;
    }

    return false;
}
