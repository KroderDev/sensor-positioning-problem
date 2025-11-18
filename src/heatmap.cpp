#include "Heatmap.hpp"
#include <opencv2/opencv.hpp>
#include <algorithm>
#include <stdexcept>
#include <string>
#include <vector>

/**
 * Genera y visualiza un mapa de calor con delimitación opcional de zonas
 * 
 * Esta función crea una visualización de mapa de calor a partir de una matriz de datos numéricos,
 * con la capacidad de mostrar zonas delimitadas por rectángulos negros. La función utiliza 
 * interpolación cúbica para generar transiciones suaves entre los valores y permite diferentes
 * niveles de resolución.
 * 
 * @param M Matriz de datos de entrada (std::vector<std::vector<float>>)
 *          Contiene los valores numéricos que se representarán como colores en el mapa.
 *          Todos los vectores internos deben tener la misma longitud.
 * 
 * @param factor Factor de escala para la resolución de la imagen resultante (int)
 *               Valores recomendados: 10-30
 *               - Valores bajos (10-15): menor detalle, procesamiento más rápido
 *               - Valores altos (25-30): mayor detalle, procesamiento más lento
 * 
 * @param Z Matriz opcional de etiquetas de zonas (std::vector<std::vector<int>>, por defecto vacía)
 *          Si se proporciona, debe tener las mismas dimensiones que M.
 *          Cada valor entero representa una zona diferente.
 *          Las zonas se delimitan automáticamente con rectángulos negros.
 *          Si se omite o está vacía, se muestra solo el mapa de calor sin delimitaciones.
 * 
 * @note Requisitos:
 *       - OpenCV debe estar instalado
 *       - La matriz M no puede estar vacía
 *       - Si se proporciona Z, debe tener las mismas dimensiones que M
 * 
 * @example Uso básico:
 * 
 *     std::vector<std::vector<float>> datos = {{1.0, 2.0}, {3.0, 4.0}};
 *     plotHeatmap(datos, 20);  // Solo mapa de calor
 * 
 * @example Uso con zonas:
 * 
 *     std::vector<std::vector<float>> datos = {{1.0, 2.0}, {3.0, 4.0}};
 *     std::vector<std::vector<int>> zonas = {{1, 2}, {1, 2}};
 *     plotHeatmap(datos, 25, zonas);  // Mapa con delimitación de zonas
 */
namespace
{
    cv::Mat buildHeatmapImage(const std::vector<std::vector<float>> &M, int factor, const std::vector<std::vector<int>> &Z)
    {
        if (M.empty() || M[0].empty()) {
            throw std::invalid_argument("La matriz de datos no puede estar vacia.");
        }

        const int rows = static_cast<int>(M.size());
        const int cols = static_cast<int>(M[0].size());

        for (const auto &row : M) {
            if (static_cast<int>(row.size()) != cols) {
                throw std::invalid_argument("Todas las filas de la matriz de datos deben tener la misma longitud.");
            }
        }

        if (!Z.empty()) {
            if (static_cast<int>(Z.size()) != rows) {
                throw std::invalid_argument("La matriz de zonas no coincide en filas con la matriz de datos.");
            }
            for (const auto &row : Z) {
                if (static_cast<int>(row.size()) != cols) {
                    throw std::invalid_argument("La matriz de zonas no coincide en columnas con la matriz de datos.");
                }
            }
        }

        cv::Mat matM(rows, cols, CV_32F);
        
        for(int i = 0; i < rows; ++i) {
            for(int j = 0; j < cols; ++j) {
                matM.at<float>(i,j) = M[i][j];
            }
        }
        
        cv::Mat matZ;
        if (!Z.empty()) {
            matZ = cv::Mat(rows, cols, CV_32S);
            for(int i = 0; i < rows; ++i) {
                for(int j = 0; j < cols; ++j) {
                    matZ.at<int>(i,j) = Z[i][j];
                }
            }
        }
        
        cv::Mat M_big;
        cv::resize(matM, M_big, cv::Size(), factor, factor, cv::INTER_CUBIC);

        cv::Mat Mnorm, heatmap;
        cv::normalize(M_big, Mnorm, 0, 255, cv::NORM_MINMAX, CV_8U);
        cv::applyColorMap(Mnorm, heatmap, cv::COLORMAP_VIRIDIS);

        if (!matZ.empty()) {
            cv::Mat heatmapRGBA;
            cv::cvtColor(heatmap, heatmapRGBA, cv::COLOR_BGR2BGRA);
            
            cv::Mat Z_big;
            cv::resize(matZ, Z_big, cv::Size(), factor, factor, cv::INTER_NEAREST);

            int borderExtension = 2;
            cv::Mat expandedHeatmap;
            cv::copyMakeBorder(heatmapRGBA, expandedHeatmap, borderExtension, borderExtension, 
                              borderExtension, borderExtension, cv::BORDER_CONSTANT, cv::Scalar(255,255,255,255));

            std::vector<int> uniqueZones;
            for(int i = 0; i < matZ.rows; ++i) {
                for(int j = 0; j < matZ.cols; ++j) {
                    int zone = matZ.at<int>(i,j);
                    if(std::find(uniqueZones.begin(), uniqueZones.end(), zone) == uniqueZones.end()) {
                        uniqueZones.push_back(zone);
                    }
                }
            }

            for(int zone : uniqueZones) {
                int minRow = matZ.rows, maxRow = -1, minCol = matZ.cols, maxCol = -1;
                
                for(int i = 0; i < matZ.rows; ++i) {
                    for(int j = 0; j < matZ.cols; ++j) {
                        if(matZ.at<int>(i,j) == zone) {
                            minRow = std::min(minRow, i);
                            maxRow = std::max(maxRow, i);
                            minCol = std::min(minCol, j);
                            maxCol = std::max(maxCol, j);
                        }
                    }
                }
                
                bool touchesTop = (minRow == 0);
                bool touchesBottom = (maxRow == matZ.rows - 1);
                bool touchesLeft = (minCol == 0);
                bool touchesRight = (maxCol == matZ.cols - 1);
                
                int margin = 2;
                int topY = minRow * factor + (touchesTop ? -borderExtension : margin) + borderExtension;
                int bottomY = (maxRow + 1) * factor - (touchesBottom ? -margin : margin) + borderExtension;
                int leftX = minCol * factor + (touchesLeft ? -borderExtension : margin) + borderExtension;
                int rightX = (maxCol + 1) * factor - (touchesRight ? -margin : margin) + borderExtension;
                
                if (touchesBottom) bottomY = std::min(bottomY + borderExtension, expandedHeatmap.rows - 1);
                if (touchesRight) rightX = std::min(rightX + borderExtension, expandedHeatmap.cols - 1);
                
                cv::Point topLeft(leftX, topY);
                cv::Point bottomRight(rightX, bottomY);
                
                cv::rectangle(expandedHeatmap, topLeft, bottomRight, cv::Scalar(0,0,0,255), 1);
            }
            
            cv::cvtColor(expandedHeatmap, heatmap, cv::COLOR_BGRA2BGR);
        }

        return heatmap;
    }
}

void plotHeatmap(const std::vector<std::vector<float>>& M, int factor, const std::vector<std::vector<int>>& Z) {
    cv::Mat heatmap = buildHeatmapImage(M, factor, Z);
    std::string windowTitle = !Z.empty() ? "Mapa de Calor con Zonas" : "Mapa de Calor";
    cv::imshow(windowTitle, heatmap);
    cv::waitKey(0);
}

void saveHeatmap(const std::vector<std::vector<float>>& M, int factor, const std::vector<std::vector<int>>& Z, const std::string &outputPath) {
    cv::Mat heatmap = buildHeatmapImage(M, factor, Z);
    if (!cv::imwrite(outputPath, heatmap)) {
        throw std::runtime_error("No se pudo escribir la imagen en " + outputPath);
    }
}
