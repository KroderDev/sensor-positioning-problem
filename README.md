# Sensor Positioning Problem (SPP) – Implementación en C++ con Simulated Annealing

Este proyecto implementa una heurística de **Simulated Annealing (SA)** para resolver el **Sensor Positioning Problem (SPP)** en el contexto de agricultura de precisión.

La idea general es:

- Dados puntos muestrales de un terreno (valores de un índice como NDVI, materia orgánica, etc.)  
- Y un conjunto de **zonas de manejo rectangulares potenciales**,  
- Se busca decidir **dónde ubicar un número limitado de sensores**, minimizando una función objetivo (típicamente, una distancia ponderada por la varianza de las zonas o alguna medida de representatividad).

La solución se construye y mejora con **Simulated Annealing** sobre una representación discreta de las ubicaciones de los sensores.

---

