# Sensor Positioning Problem (SPP) – Implementación en C++ con Simulated Annealing

Este proyecto implementa una heurística de **Simulated Annealing (SA)** para resolver el **Sensor Positioning Problem (SPP)** en el contexto de agricultura de precisión.

La idea general es:

- Dados puntos muestrales de un terreno (valores de un índice como NDVI, materia orgánica, etc.)
- Y un conjunto de **zonas de manejo rectangulares potenciales**,
- Se busca decidir **dónde ubicar un número limitado de sensores**, minimizando una función objetivo (típicamente, una distancia ponderada por la varianza de las zonas o alguna medida de representatividad).

La solución se construye y mejora con **Simulated Annealing** sobre una representación discreta de las ubicaciones de los sensores.

---

## Funcionamiento del algoritmo

- **Entrada**: instancia `N x M` con matriz `S`, cantidad de zonas/sensores `p`, nivel de homogeneidad `alpha`, y parametros SA (`data/config/default.json`).
- **Inicializacion**: se generan `p` zonas mediante cortes guillotina aleatorios (rectangulos disjuntos) y se calcula el error cuadratico total. Si la particion no respeta conexidad/varianza se prueban vecinos hasta hallar una solucion valida.
- **Vecindario**: mueve una franja de borde entre zonas adyacentes (expandir/contraer). Tras aceptar un vecino se intenta reparar para que cada zona sea su bounding box si los rectangulos no se solapan; si no es posible, se descarta.
- **Evaluacion**: energia = error cuadratico total + `penalty_weight * exceso_de_varianza`. La varianza que excede `alpha * Var(S)` se penaliza, permitiendo explorar soluciones infeasibles mientras tengan buena energia.
- **Aceptacion SA**: mejoras se aceptan siempre; empeoramientos con probabilidad `exp(-delta/temperatura)`. Se usan limites de iteraciones, tiempo y temperatura (`T0`, `Tf`, `cooling_factor`, `iters_per_temp`, `max_iterations`, `max_time_seconds`).
- **Salida**: se escriben dos archivos en `data/solutions/`: `{instancia}_initial.out` (antes de SA) y `{instancia}_best.out` (mejor solucion encontrada).
