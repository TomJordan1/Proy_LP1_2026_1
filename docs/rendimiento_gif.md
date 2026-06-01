# Análisis de rendimiento del GIF — Simulador V2

## 1. Aclaración clave: el hashing NO tiene que ver con la lentitud del GIF

Conviene separar dos etapas que ocurren en momentos distintos y en lenguajes distintos:

| Etapa | Dónde | Costo | Estado |
|-------|-------|-------|--------|
| Simulación física (movimiento + colisiones) | C (`Opti.c`, spatial grid hashing) | **O(N)** por tick → O(N·ticks) total | Rápido y ya optimizado |
| Renderizado del GIF | Python (`matplotlib` + `pillow`) | O(frames · N) + costo fijo alto por frame | **Este es el cuello de botella** |

El *spatial hashing* es una técnica de la **simulación**: evita comparar todas las parejas de partículas (O(N²)) y reduce la detección de colisiones a O(N). Tu benchmark lo confirma: con 10 → 100 → 1000 partículas el tiempo de C crece de forma **lineal** (0.012s → 0.110s → ~1.0s), siguiendo la recta O(N), no la parábola O(N²).

Por lo tanto, **optimizar más el hashing no acelera el GIF**: son problemas independientes. El GIF es lento por cómo `matplotlib` dibuja cada cuadro, no por la física.

## 2. Por qué el GIF crece "exponencialmente" (en realidad, multiplicativo)

No es exponencial en sentido estricto; es **multiplicativo**: el tiempo de render ≈ `n_frames × costo_por_frame`, y ambos factores crecen al subir partículas y ticks.

En el `animador_particulas.py` original hay tres causas concretas:

1. **Un frame por cada tick.** Con `ticks=1000` se renderizan 1000 cuadros. Para una animación fluida bastan 60–150. Renderizar 1000 cuadros es ~10× más trabajo del necesario.

2. **`df[df['tick'] == tick]` dentro de `update`.** Esto recorre **todo** el DataFrame en cada frame. Con 100 part × 1000 ticks = 100 000 filas, son 100 000 comparaciones × 1000 frames = **10⁸ operaciones solo en filtrado**. Es un patrón O(frames × filas).

3. **`ax.clear()` + `iterrows()` + un `Circle`/`quiver`/`text` por partícula en cada frame.** `iterrows()` es la forma más lenta de recorrer un DataFrame, y crear objetos `Circle` nuevos por partícula y por frame obliga a `matplotlib` a reconstruir toda la escena. Esto domina el tiempo.

Multiplicando: 1000 frames × (filtro lento + N objetos nuevos + redibujo completo) = minutos para 100 part / 1000 ticks.

## 3. Qué se hizo en el nuevo `visualizar.py`

El nuevo script ya incorpora las optimizaciones estándar (sin tocar tu `animador_particulas.py`):

- **`groupby('tick')` una sola vez** → cada frame accede a su grupo en O(1), elimina el filtrado repetido (causa 2).
- **Un único `scatter`** reutilizado con `set_offsets` / `set_sizes` / `set_color`, en lugar de recrear `Circle` por partícula (causa 3). El radio se mapea al área del marcador.
- **Submuestreo a `max_frames=120`** cuadros como máximo (causa 1).
- **Backend `Agg`** (sin ventana) y opción `--sin-gif` para generar solo las gráficas estáticas cuando el GIF no hace falta.

Con esto, 100 part / 1000 ticks pasa de minutos a unos pocos segundos.

## 4. Tecnologías más eficientes que GIF + pillow (lo que pediste investigar)

El formato GIF en sí es parte del problema: está limitado a 256 colores, comprime mal y `pillow` ensambla los cuadros en memoria. Alternativas modernas, de menor a mayor esfuerzo:

1. **Blitting de matplotlib** (`FuncAnimation(..., blit=True)`): solo redibuja los artistas que cambian (el scatter), no los ejes ni la grilla. 2–5× más rápido. Requiere que `update` devuelva los artistas modificados y que ejes/título sean fijos.

2. **Exportar a MP4/WebM con FFmpeg** en vez de GIF (`writer='ffmpeg'`). El vídeo H.264 pesa ~5–10× menos que el GIF equivalente, admite color completo y FFmpeg codifica en streaming (no acumula todos los cuadros en RAM). Es el estándar actual para animaciones científicas. Solo necesita tener `ffmpeg` instalado.

3. **`matplotlib.animation` + `tight`/`FFMpegWriter` con `dpi` reducido**: bajar dpi de 90 a ~70 reduce los píxeles a procesar por cuadro casi a la mitad.

4. **Renderizado por GPU / datasets grandes**: si algún día quieres miles de partículas en tiempo real, lo idóneo es saltar de matplotlib a:
   - **`vispy`** o **`pyqtgraph`** (OpenGL): dibujan cientos de miles de puntos a 60 fps; pensados justo para nubes de partículas.
   - **`datashader`** para densidades enormes (rasteriza millones de puntos), excelente para el heatmap si N crece mucho.

5. **Idea de arquitectura**: el GIF es para *presentar*, no para analizar. Lo más eficiente es generar el GIF solo a baja resolución/pocos frames para la exposición, y usar las gráficas estáticas (energía, heatmap, benchmark) — que son baratas — para el análisis cuantitativo.

## 5. Recomendación práctica

- Para entregas y exposición: GIF submuestreado (≤120 frames) con el nuevo `visualizar.py`, o mejor un **MP4 con FFmpeg**.
- Para N grande o muchos ticks: usar `--sin-gif` y quedarte con las gráficas PNG; si necesitas animación interactiva, migrar a `pyqtgraph`/`vispy`.
- El spatial hashing de C ya está bien; no es ahí donde está el tiempo.
