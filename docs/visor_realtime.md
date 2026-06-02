# Visor en tiempo real con pyqtgraph (GPU/OpenGL)

`src/visualizar_realtime.py` reemplaza la generación lenta de GIF por una
**reproducción interactiva en vivo** de la simulación, dibujada sobre Qt con
aceleración OpenGL.

## Por qué es más eficiente que el GIF de matplotlib

- matplotlib rasteriza cada cuadro en CPU (backend Agg) y, en el animador
  original, recreaba un objeto por partícula en cada frame. Por eso 100
  partículas × 1000 ticks tardaba minutos.
- pyqtgraph usa un **único `ScatterPlotItem`** que se actualiza con `setData()`;
  no recrea objetos ni rasteriza en CPU. Con OpenGL activo dibuja decenas o
  cientos de miles de puntos a ~60 fps.
- Los datos se preprocesan **una sola vez** (`groupby("tick")` → arrays NumPy por
  tick), así que en reproducción no se filtra el DataFrame en cada cuadro.

## Instalación

```bash
pip install pyqtgraph PyQt5 numpy pandas
# opcional, solo si vas a exportar MP4:
pip install imageio imageio-ffmpeg
```

## Uso

```bash
# Visor interactivo (busca trayectorias.csv en ./, target/, c_modules/)
python visualizar_realtime.py

# Indicando archivo y dimensiones del mundo
python visualizar_realtime.py trayectorias.csv --ancho 1000 --alto 1000

# Submuestreo y velocidad de reproducción
python visualizar_realtime.py trayectorias.csv --salto 3 --fps 60

# Exportar a MP4 sin abrir ventana (headless, streaming a FFmpeg)
python visualizar_realtime.py trayectorias.csv --exportar simulacion.mp4 --fps 30
```

## Estilo: flechas de velocidad e ids

Tanto el visor como el GIF de `visualizar.py` permiten elegir el estilo de dibujo.
Al ejecutar sin banderas aparece un menu:

```
1. Simple        - circulos por radio
2. Con flechas   - circulos + vector de velocidad (flecha roja)
3. Flechas + ids - circulos + flechas + numero de particula
```

Tambien por bandera (sin menu):

```bash
python visualizar_realtime.py trayectorias.csv --flechas
python visualizar_realtime.py trayectorias.csv --flechas --ids
python visualizar_realtime.py trayectorias.csv --simple
```

En el GIF es igual:

```bash
python visualizar.py --flechas --ids
python visualizar.py --simple
```

El tamano de cada circulo siempre es proporcional al radio real de la particula.
Nota: la exportacion a MP4 del visor (`--exportar`) dibuja solo los circulos;
si necesitas un archivo CON flechas, usa el GIF de `visualizar.py --flechas`.

## Controles del visor

| Tecla / control | Acción |
|-----------------|--------|
| Espacio / botón Pausa | Play / pausa |
| Flecha derecha / izquierda | Avanzar / retroceder 1 cuadro (en pausa) |
| R / botón Reiniciar | Volver al tick 0 |
| Slider | Saltar a cualquier tick |
| Spin "Velocidad" | fps de reproducción |
| Spin "Salto" | submuestreo (1 de cada k ticks) |

## Reparto de tareas de visualización

- `visualizar.py` → gráficas estáticas (energía, heatmap, benchmark) + GIF para entregas.
- `visualizar_realtime.py` → exploración interactiva en vivo y export MP4 (más rápido y ligero que GIF).

## Cuándo pasar a vispy / OpenGL puro

pyqtgraph cubre con holgura el rango de este proyecto (decenas a miles de
partículas). Si en el futuro se quisieran **cientos de miles o millones** de
partículas en tiempo real, lo indicado es:

- **vispy**: API OpenGL de alto nivel, *point sprites* en GPU; pensado justo
  para nubes de partículas masivas.
- **datashader**: para densidades enormes (rasteriza millones de puntos),
  ideal si el heatmap creciera mucho.
- **Renderizar en el propio C** (SDL2 / raylib) volcando frames a FFmpeg:
  elimina Python del bucle de animación; es el camino de los motores de
  partículas profesionales.

Esto queda como **propuesta de optimización futura** del proyecto.
