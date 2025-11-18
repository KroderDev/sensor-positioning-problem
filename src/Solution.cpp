#include "Solution.hpp"

#include <algorithm>
#include <cmath>
#include <queue>
#include <random>

namespace
{
    using Grid = std::vector<std::vector<int>>;

    struct Bounds
    {
        int top = 0;
        int bottom = 0;
        int left = 0;
        int right = 0;
        bool initialized = false;
    };

    std::vector<Bounds> computeZoneBounds(const Grid &Z, int p)
    {
        std::vector<Bounds> bounds(p + 1);

        for (int i = 0; i < static_cast<int>(Z.size()); ++i)
        {
            for (int j = 0; j < static_cast<int>(Z[0].size()); ++j)
            {
                int zone = Z[i][j];
                auto &b = bounds[zone];
                if (!b.initialized)
                {
                    b.top = b.bottom = i;
                    b.left = b.right = j;
                    b.initialized = true;
                }
                else
                {
                    b.top = std::min(b.top, i);
                    b.bottom = std::max(b.bottom, i);
                    b.left = std::min(b.left, j);
                    b.right = std::max(b.right, j);
                }
            }
        }

        return bounds;
    }

}

Solution buildInitialSolution(const ProblemInstance &instance)
{
    Solution sol;
    sol.Z.assign(instance.nRows, std::vector<int>(instance.nCols, 1));

    struct Rect
    {
        int top;
        int bottom;
        int left;
        int right;

        int height() const { return bottom - top + 1; }
        int width() const { return right - left + 1; }
        int area() const { return height() * width(); }
    };

    std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<double> uniform01(0.0, 1.0);

    std::vector<Rect> rects;
    rects.push_back(Rect{0, instance.nRows - 1, 0, instance.nCols - 1});

    // Generamos p rectangulos mediante cortes aleatorios (guillotine), garantizando p zonas
    while (static_cast<int>(rects.size()) < instance.p)
    {
        // Elegir rectangulo a dividir: el de mayor area para evitar celdas demasiado pequenas
        int idx = 0;
        for (int i = 1; i < static_cast<int>(rects.size()); ++i)
        {
            if (rects[i].area() > rects[idx].area())
            {
                idx = i;
            }
        }

        Rect r = rects[idx];
        bool canSplitH = r.height() >= 2;
        bool canSplitV = r.width() >= 2;
        if (!canSplitH && !canSplitV)
        {
            break; // no se puede seguir dividiendo
        }

        bool splitHorizontal = false;
        if (canSplitH && canSplitV)
        {
            splitHorizontal = uniform01(rng) < 0.5;
        }
        else if (canSplitH)
        {
            splitHorizontal = true;
        }

        if (splitHorizontal)
        {
            std::uniform_int_distribution<int> cutDist(r.top, r.bottom - 1);
            int cut = cutDist(rng);
            Rect topRect{r.top, cut, r.left, r.right};
            Rect bottomRect{cut + 1, r.bottom, r.left, r.right};
            rects[idx] = topRect;
            rects.push_back(bottomRect);
        }
        else
        {
            std::uniform_int_distribution<int> cutDist(r.left, r.right - 1);
            int cut = cutDist(rng);
            Rect leftRect{r.top, r.bottom, r.left, cut};
            Rect rightRect{r.top, r.bottom, cut + 1, r.right};
            rects[idx] = leftRect;
            rects.push_back(rightRect);
        }
    }

    // Asignar zonas 1..p a los rectangulos generados (si sobran zonas, las dos ultimas se fusionan)
    while (static_cast<int>(rects.size()) > instance.p)
    {
        rects.pop_back();
    }

    for (int zoneId = 1; zoneId <= static_cast<int>(rects.size()); ++zoneId)
    {
        const Rect &r = rects[zoneId - 1];
        for (int i = r.top; i <= r.bottom; ++i)
        {
            for (int j = r.left; j <= r.right; ++j)
            {
                sol.Z[i][j] = zoneId;
            }
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

double calculateVariancePenalty(const ProblemInstance &instance,
                                const std::vector<double> &variances,
                                const std::vector<int> &counts,
                                double totalVariance)
{
    double maxVariance = instance.alpha * totalVariance;
    double penalty = 0.0;
    for (int k = 1; k <= instance.p; ++k)
    {
        if (counts[k] == 0)
        {
            // Zona vacía: penalizar fuertemente
            penalty += maxVariance;
            continue;
        }
        if (variances[k] > maxVariance)
        {
            penalty += variances[k] - maxVariance;
        }
    }
    return penalty;
}

bool isSolutionValid(const ProblemInstance &instance,
                     const std::vector<std::vector<int>> &Z,
                     double totalVariance)
{
    std::vector<double> means(instance.p + 1, 0.0), variances(instance.p + 1, 0.0);
    std::vector<int> counts(instance.p + 1, 0);
    calculateErrorAndVariance(instance, Z, means, variances, counts);

    // Chequear que cada zona es conexa (4-neighbors) y respeta la varianza máxima.
    const int nRows = instance.nRows;
    const int nCols = instance.nCols;
    std::vector<std::vector<bool>> visited(nRows, std::vector<bool>(nCols, false));

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

        // BFS para verificar conexidad de la zona k.
        int startR = -1, startC = -1;
        for (int i = 0; i < nRows && startR == -1; ++i)
        {
            for (int j = 0; j < nCols; ++j)
            {
                if (Z[i][j] == k)
                {
                    startR = i;
                    startC = j;
                    break;
                }
            }
        }
        if (startR == -1)
        {
            return false;
        }

        int visitedCount = 0;
        std::queue<std::pair<int, int>> q;
        q.push({startR, startC});
        visited[startR][startC] = true;

        const int dr[4] = {-1, 1, 0, 0};
        const int dc[4] = {0, 0, -1, 1};
        while (!q.empty())
        {
            auto [r, c] = q.front();
            q.pop();
            ++visitedCount;

            for (int dir = 0; dir < 4; ++dir)
            {
                int nr = r + dr[dir];
                int nc = c + dc[dir];
                if (nr < 0 || nr >= nRows || nc < 0 || nc >= nCols)
                {
                    continue;
                }
                if (visited[nr][nc] || Z[nr][nc] != k)
                {
                    continue;
                }
                visited[nr][nc] = true;
                q.push({nr, nc});
            }
        }

        if (visitedCount != counts[k])
        {
            return false;
        }
    }
    return true;
}

bool isPartitionConnected(const ProblemInstance &instance,
                          const std::vector<std::vector<int>> &Z)
{
    const int nRows = instance.nRows;
    const int nCols = instance.nCols;

    std::vector<int> counts(instance.p + 1, 0);
    std::vector<std::vector<bool>> visited(nRows, std::vector<bool>(nCols, false));

    for (int i = 0; i < nRows; ++i)
    {
        for (int j = 0; j < nCols; ++j)
        {
            int z = Z[i][j];
            if (z < 1 || z > instance.p)
            {
                return false;
            }
            counts[z]++;
        }
    }

    const int dr[4] = {-1, 1, 0, 0};
    const int dc[4] = {0, 0, -1, 1};

    for (int k = 1; k <= instance.p; ++k)
    {
        if (counts[k] == 0)
        {
            return false;
        }
        int startR = -1, startC = -1;
        for (int i = 0; i < nRows && startR == -1; ++i)
        {
            for (int j = 0; j < nCols; ++j)
            {
                if (Z[i][j] == k)
                {
                    startR = i;
                    startC = j;
                    break;
                }
            }
        }
        if (startR == -1)
        {
            return false;
        }

        int visitedCount = 0;
        std::queue<std::pair<int, int>> q;
        q.push({startR, startC});
        visited[startR][startC] = true;

        while (!q.empty())
        {
            auto [r, c] = q.front();
            q.pop();
            ++visitedCount;

            for (int dir = 0; dir < 4; ++dir)
            {
                int nr = r + dr[dir];
                int nc = c + dc[dir];
                if (nr < 0 || nr >= nRows || nc < 0 || nc >= nCols)
                {
                    continue;
                }
                if (visited[nr][nc] || Z[nr][nc] != k)
                {
                    continue;
                }
                visited[nr][nc] = true;
                q.push({nr, nc});
            }
        }

        if (visitedCount != counts[k])
        {
            return false;
        }
    }

    return true;
}

// Intenta reparar la partición forzando que cada zona sea exactamente su bounding box.
// Solo funciona si los rectángulos no se solapan entre sí. Devuelve true si se pudo reparar.
bool makeRectsIfNonOverlapping(const ProblemInstance &instance,
                               std::vector<std::vector<int>> &Z)
{
    const auto bounds = computeZoneBounds(Z, instance.p);
    for (int k = 1; k <= instance.p; ++k)
    {
        if (!bounds[k].initialized)
        {
            return false;
        }
    }

    // Verificar solapamientos entre bounding boxes
    for (int a = 1; a <= instance.p; ++a)
    {
        for (int b = a + 1; b <= instance.p; ++b)
        {
            const auto &ba = bounds[a];
            const auto &bb = bounds[b];
            bool overlap = !(ba.right < bb.left || bb.right < ba.left || ba.bottom < bb.top || bb.bottom < ba.top);
            if (overlap)
            {
                return false;
            }
        }
    }

    // Sin solapes: podemos rellenar cada rectángulo
    for (int k = 1; k <= instance.p; ++k)
    {
        const auto &b = bounds[k];
        for (int r = b.top; r <= b.bottom; ++r)
        {
            for (int c = b.left; c <= b.right; ++c)
            {
                Z[r][c] = k;
            }
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

    auto bounds = computeZoneBounds(currentZ, instance.p);
    std::uniform_int_distribution<int> zoneDist(1, instance.p);
    std::uniform_int_distribution<int> dirDist(0, 3);    // 0:top,1:bottom,2:left,3:right
    std::uniform_int_distribution<int> expandDist(0, 1); // 1 expand, 0 shrink

    auto pickUnique = [](std::vector<int> &vec, int val) {
        if (std::find(vec.begin(), vec.end(), val) == vec.end())
        {
            vec.push_back(val);
        }
    };

    for (int attempt = 0; attempt < 200; ++attempt)
    {
        neighborZ = currentZ;
        int zone = zoneDist(rng);
        const auto &b = bounds[zone];
        if (!b.initialized)
        {
            continue;
        }

        int dir = dirDist(rng);
        bool expand = expandDist(rng) == 1;
        bool success = false;

        if (dir == 0) // mover borde superior
        {
            if (expand)
            {
                int targetRow = b.top - 1;
                if (targetRow < 0)
                {
                    continue;
                }
                std::vector<int> adjZones;
                for (int c = b.left; c <= b.right; ++c)
                {
                    int adj = neighborZ[targetRow][c];
                    if (adj > 0 && adj != zone)
                    {
                        pickUnique(adjZones, adj);
                    }
                }
                if (adjZones.empty())
                {
                    continue;
                }
                std::uniform_int_distribution<int> adjDist(0, static_cast<int>(adjZones.size()) - 1);
                int adj = adjZones[adjDist(rng)];
                const auto &adjB = bounds[adj];
                if (!adjB.initialized || adjB.bottom < targetRow)
                {
                    continue;
                }
                int c1 = std::max(b.left, adjB.left);
                int c2 = std::min(b.right, adjB.right);
                if (c1 > c2)
                {
                    continue;
                }
                for (int c = c1; c <= c2; ++c)
                {
                    neighborZ[targetRow][c] = zone;
                }
                success = true;
            }
            else
            {
                if (b.top == 0 || b.top == b.bottom)
                {
                    continue;
                }
                int targetRow = b.top - 1;
                std::vector<int> adjZones;
                for (int c = b.left; c <= b.right; ++c)
                {
                    int adj = neighborZ[targetRow][c];
                    if (adj > 0 && adj != zone)
                    {
                        pickUnique(adjZones, adj);
                    }
                }
                if (adjZones.empty())
                {
                    continue;
                }
                std::uniform_int_distribution<int> adjDist(0, static_cast<int>(adjZones.size()) - 1);
                int adj = adjZones[adjDist(rng)];
                const auto &adjB = bounds[adj];
                if (!adjB.initialized || adjB.bottom < targetRow)
                {
                    continue;
                }
                int c1 = std::max(b.left, adjB.left);
                int c2 = std::min(b.right, adjB.right);
                if (c1 > c2)
                {
                    continue;
                }
                for (int c = c1; c <= c2; ++c)
                {
                    neighborZ[b.top][c] = adj;
                }
                success = true;
            }
        }
        else if (dir == 1) // borde inferior
        {
            if (expand)
            {
                int targetRow = b.bottom + 1;
                if (targetRow >= nRows)
                {
                    continue;
                }
                std::vector<int> adjZones;
                for (int c = b.left; c <= b.right; ++c)
                {
                    int adj = neighborZ[targetRow][c];
                    if (adj > 0 && adj != zone)
                    {
                        pickUnique(adjZones, adj);
                    }
                }
                if (adjZones.empty())
                {
                    continue;
                }
                std::uniform_int_distribution<int> adjDist(0, static_cast<int>(adjZones.size()) - 1);
                int adj = adjZones[adjDist(rng)];
                const auto &adjB = bounds[adj];
                if (!adjB.initialized || adjB.top > targetRow)
                {
                    continue;
                }
                int c1 = std::max(b.left, adjB.left);
                int c2 = std::min(b.right, adjB.right);
                if (c1 > c2)
                {
                    continue;
                }
                for (int c = c1; c <= c2; ++c)
                {
                    neighborZ[targetRow][c] = zone;
                }
                success = true;
            }
            else
            {
                if (b.bottom == nRows - 1 || b.top == b.bottom)
                {
                    continue;
                }
                int targetRow = b.bottom + 1;
                std::vector<int> adjZones;
                for (int c = b.left; c <= b.right; ++c)
                {
                    int adj = neighborZ[targetRow][c];
                    if (adj > 0 && adj != zone)
                    {
                        pickUnique(adjZones, adj);
                    }
                }
                if (adjZones.empty())
                {
                    continue;
                }
                std::uniform_int_distribution<int> adjDist(0, static_cast<int>(adjZones.size()) - 1);
                int adj = adjZones[adjDist(rng)];
                const auto &adjB = bounds[adj];
                if (!adjB.initialized || adjB.top > targetRow)
                {
                    continue;
                }
                int c1 = std::max(b.left, adjB.left);
                int c2 = std::min(b.right, adjB.right);
                if (c1 > c2)
                {
                    continue;
                }
                for (int c = c1; c <= c2; ++c)
                {
                    neighborZ[b.bottom][c] = adj;
                }
                success = true;
            }
        }
        else if (dir == 2) // borde izquierdo
        {
            if (expand)
            {
                int targetCol = b.left - 1;
                if (targetCol < 0)
                {
                    continue;
                }
                std::vector<int> adjZones;
                for (int r = b.top; r <= b.bottom; ++r)
                {
                    int adj = neighborZ[r][targetCol];
                    if (adj > 0 && adj != zone)
                    {
                        pickUnique(adjZones, adj);
                    }
                }
                if (adjZones.empty())
                {
                    continue;
                }
                std::uniform_int_distribution<int> adjDist(0, static_cast<int>(adjZones.size()) - 1);
                int adj = adjZones[adjDist(rng)];
                const auto &adjB = bounds[adj];
                if (!adjB.initialized || adjB.right < targetCol)
                {
                    continue;
                }
                int r1 = std::max(b.top, adjB.top);
                int r2 = std::min(b.bottom, adjB.bottom);
                if (r1 > r2)
                {
                    continue;
                }
                for (int r = r1; r <= r2; ++r)
                {
                    neighborZ[r][targetCol] = zone;
                }
                success = true;
            }
            else
            {
                if (b.left == 0 || b.left == b.right)
                {
                    continue;
                }
                int targetCol = b.left - 1;
                std::vector<int> adjZones;
                for (int r = b.top; r <= b.bottom; ++r)
                {
                    int adj = neighborZ[r][targetCol];
                    if (adj > 0 && adj != zone)
                    {
                        pickUnique(adjZones, adj);
                    }
                }
                if (adjZones.empty())
                {
                    continue;
                }
                std::uniform_int_distribution<int> adjDist(0, static_cast<int>(adjZones.size()) - 1);
                int adj = adjZones[adjDist(rng)];
                const auto &adjB = bounds[adj];
                if (!adjB.initialized || adjB.right < targetCol)
                {
                    continue;
                }
                int r1 = std::max(b.top, adjB.top);
                int r2 = std::min(b.bottom, adjB.bottom);
                if (r1 > r2)
                {
                    continue;
                }
                for (int r = r1; r <= r2; ++r)
                {
                    neighborZ[r][b.left] = adj;
                }
                success = true;
            }
        }
        else // borde derecho
        {
            if (expand)
            {
                int targetCol = b.right + 1;
                if (targetCol >= nCols)
                {
                    continue;
                }
                std::vector<int> adjZones;
                for (int r = b.top; r <= b.bottom; ++r)
                {
                    int adj = neighborZ[r][targetCol];
                    if (adj > 0 && adj != zone)
                    {
                        pickUnique(adjZones, adj);
                    }
                }
                if (adjZones.empty())
                {
                    continue;
                }
                std::uniform_int_distribution<int> adjDist(0, static_cast<int>(adjZones.size()) - 1);
                int adj = adjZones[adjDist(rng)];
                const auto &adjB = bounds[adj];
                if (!adjB.initialized || adjB.left > targetCol)
                {
                    continue;
                }
                int r1 = std::max(b.top, adjB.top);
                int r2 = std::min(b.bottom, adjB.bottom);
                if (r1 > r2)
                {
                    continue;
                }
                for (int r = r1; r <= r2; ++r)
                {
                    neighborZ[r][targetCol] = zone;
                }
                success = true;
            }
            else
            {
                if (b.right == nCols - 1 || b.left == b.right)
                {
                    continue;
                }
                int targetCol = b.right + 1;
                std::vector<int> adjZones;
                for (int r = b.top; r <= b.bottom; ++r)
                {
                    int adj = neighborZ[r][targetCol];
                    if (adj > 0 && adj != zone)
                    {
                        pickUnique(adjZones, adj);
                    }
                }
                if (adjZones.empty())
                {
                    continue;
                }
                std::uniform_int_distribution<int> adjDist(0, static_cast<int>(adjZones.size()) - 1);
                int adj = adjZones[adjDist(rng)];
                const auto &adjB = bounds[adj];
                if (!adjB.initialized || adjB.left > targetCol)
                {
                    continue;
                }
                int r1 = std::max(b.top, adjB.top);
                int r2 = std::min(b.bottom, adjB.bottom);
                if (r1 > r2)
                {
                    continue;
                }
                for (int r = r1; r <= r2; ++r)
                {
                    neighborZ[r][b.right] = adj;
                }
                success = true;
            }
        }

        if (success)
        {
            return true;
        }
    }

    return false;
}
