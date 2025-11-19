# Sensor Positioning Problem (SPP) – Implementación en C++ con Simulated Annealing

Este proyecto implementa una heurística de **Simulated Annealing (SA)** para resolver el **Sensor Positioning Problem (SPP)** en agricultura de precisión.

El objetivo es dividir un terreno agrícola (dado por una matriz de datos como NDVI o pH) en $p$ **zonas de manejo rectangulares**, buscando minimizar la varianza interna de cada zona para asegurar homogeneidad.

-----

## Funcionamiento del Algoritmo

El proceso consta de dos etapas principales visualizadas a continuación:

### 1\. Solución Inicial

Para comenzar con una configuración válida geométricamente, el algoritmo divide el terreno recursivamente mediante cortes aleatorios (horizontales o verticales) hasta obtener las $p$ zonas deseadas.

### 2\. Optimización

Una vez generada la solución inicial, el **Simulated Annealing** refina las zonas iterativamente:

  - **Movimiento:** Selecciona una zona y mueve sus fronteras (expandiendo o contrayendo) hacia una dirección aleatoria.
  - **Reparación:** Si el movimiento genera una forma irregular, se intenta ajustar la zona a su *Bounding Box* para mantener la restricción de forma rectangular.
  - **Evaluación:** Se penalizan las soluciones cuya varianza exceda el umbral $\alpha$.

> [!NOTE]
> Para detalles profundos sobre la formulación matemática, pseudocódigos y análisis de resultados, consulta la [Presentación del Proyecto](docs/Presentation.md) (disponible también en PDF y HTML en la carpeta `docs/`).

-----

## Requerimientos

  - **C++17** y `make`.
  - **OpenCV 4** (necesario para la visualización y exportación de heatmaps).
      - Debian/Ubuntu: `sudo apt install libopencv-dev`

## Instalación y ejecución

1.  **Compilar:**
    ```bash
    make
    ```
2.  **Ejecutar:**
    ```bash
    make run
    ```
    *El programa solicitará por consola la cantidad de zonas `p` y el factor de homogeneidad `alpha`.*

## Salidas

Los resultados se generan en `data/solutions/`:

  - `{instancia}_initial.out`: Solución de partida.
  - `{instancia}_best.out`: Mejor solución encontrada (matriz de etiquetas).
  - `{instancia}_best.out.png`: Heatmap visualizando la matriz de datos y las zonas resultantes.