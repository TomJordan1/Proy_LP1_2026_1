# Arquitectura y Análisis de Rendimiento

Este documento describe las principales decisiones de diseño del simulador **IMPACTA**, orientadas a buscar un balance adecuado entre la física computacional y la generación de recursos visuales.

## 1. Simulación Física (C) vs. Renderizado (Python)

El proyecto separa el cálculo numérico del renderizado gráfico para intentar mitigar posibles cuellos de botella cruzados:

| Etapa | Lenguaje | Complejidad Estimada | Notas sobre Optimización |
|-------|----------|----------------------|--------------------------|
| **Física (Colisiones y cinemática)** | C | **O(N)** por tick | Emplea *Spatial Grid Hashing*, lo que ayuda a reducir comprobaciones innecesarias en comparación con un enfoque tradicional O(N²). |
| **Generación Visual (GIF/Gráficas)** | Python (`matplotlib`) | **O(frames · N)** | Depende en gran medida de la rasterización en CPU. |

Gracias a la partición espacial, el tiempo de ejecución en C tiende a comportarse de forma lineal respecto al número de partículas, lo que facilita simular escenarios amplios en tiempos manejables. Por otro lado, la etapa de renderizado visual suele requerir una parte más considerable de los recursos del programa.

## 2. Optimizaciones en la Visualización Gráfica (`visualizar.py`)

Para mejorar los tiempos de respuesta de `matplotlib` durante la creación de recursos como el GIF o el mapa de calor (Heatmap), se incorporaron algunas estrategias en el módulo de Python:

1. **Pre-procesamiento con `groupby`:** Se optó por agrupar los datos exportados (`trayectorias.csv`) mediante la columna `tick` al inicio (`df.groupby('tick')`). Esto ayuda a agilizar las consultas por fotograma, evitando iterar todo el DataFrame en cada paso.
2. **Reutilización de Objetos (`Scatter`):** En lugar de instanciar un objeto geométrico individual por cada partícula y en cada fotograma, se utiliza una única colección de puntos (`scatter`). Al actualizar únicamente los vectores de posición y escalar los puntos según el radio, se evita la sobrecarga de crear y destruir miles de objetos en memoria, optimizando notablemente el renderizado.
3. **Submuestreo de Fotogramas:** En simulaciones prolongadas, el programa puede aplicar un submuestreo para generar un número acotado de cuadros (por ejemplo, 120). Esto contribuye a obtener animaciones razonablemente fluidas cuidando el uso de memoria.
4. **Uso del Backend `Agg`:** Al ejecutar los scripts de forma automatizada, se invoca el *backend* `Agg`. Este enfoque, al procesar los gráficos en memoria y sin ventanas, suele favorecer la velocidad general del proceso.

## 3. Consideraciones y Posibles Mejoras a Futuro

- **Formatos de video alternativos:** Aunque en este momento se utiliza `.gif` para las animaciones, en escenarios de mayor escala podría resultar conveniente explorar la exportación a `.mp4` (mediante herramientas como FFmpeg), ya que tiende a ofrecer mejor compresión de archivos.
- **Visualización interactiva y rendimiento gráfico:** Para simulaciones en tiempo real o con volúmenes de partículas muy elevados, `matplotlib` podría volverse una limitante. Como paso a futuro, se podría explorar la integración de librerías como **Raylib** (que el profesor mencionó como una opción potente, permitiendo un manejo gráfico directo y eficiente desde C) o herramientas basadas en aceleración por GPU.
