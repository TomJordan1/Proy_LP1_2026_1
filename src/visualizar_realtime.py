#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
visualizar_realtime.py — Visor de particulas en TIEMPO REAL con pyqtgraph (GPU/OpenGL)
Curso: SW-303 Lenguaje de Programacion I — Imperativo, Lenguaje C
Repo: https://github.com/TomJordan1/Proy_LP1_2026_1

Por que pyqtgraph y no matplotlib:
    matplotlib rasteriza cada cuadro en CPU (backend Agg), por eso generar el GIF
    es lento. pyqtgraph dibuja sobre Qt y puede usar OpenGL (GPU), redibujando
    miles de particulas a 60 fps sin recrear objetos por cuadro. Aqui NO se
    "genera" un archivo cuadro a cuadro: se reproduce la simulacion en vivo
    actualizando un unico ScatterPlotItem con setData() en cada tick.

Lee:
    trayectorias.csv  con columnas:  tick,id,x,y,vx,vy,radio   (esquema V2)

Controles:
    Espacio  -> play / pausa
    Flecha derecha / izquierda -> avanzar / retroceder 1 cuadro (en pausa)
    R        -> reiniciar al tick 0
    Slider   -> saltar a cualquier tick
    Spin "Velocidad" -> cuadros por segundo de reproduccion
    Spin "Salto"     -> submuestreo (dibuja 1 de cada k ticks)

Uso:
    python visualizar_realtime.py
    python visualizar_realtime.py trayectorias.csv
    python visualizar_realtime.py trayectorias.csv --ancho 1000 --alto 1000
    python visualizar_realtime.py trayectorias.csv --exportar salida.mp4   # render headless a video

Requisitos:
    pip install pyqtgraph PyQt5 numpy pandas
    # exportar MP4 ademas: pip install imageio imageio-ffmpeg
"""

import argparse
import os
import sys

import numpy as np
import pandas as pd
import pyqtgraph as pg
from pyqtgraph.Qt import QtCore, QtWidgets


# =============================================================
# CARGA Y PRE-PROCESADO DE DATOS
# =============================================================
def cargar_datos(csv_path):
    """
    Carga trayectorias.csv y lo deja indexado por tick para acceso O(1).

    Se preprocesa UNA sola vez: para cada tick guardamos arrays NumPy de
    posiciones, radios y colores. En reproduccion no se vuelve a filtrar el
    DataFrame (ese era el cuello de botella del animador con matplotlib).
    """
    if not os.path.exists(csv_path):
        print(f"Error: no se encontro '{csv_path}'.")
        sys.exit(1)

    df = pd.read_csv(csv_path, encoding="utf-8")
    df = df.sort_values(["tick", "id"])

    ids = np.sort(df["id"].unique())
    n_part = len(ids)

    # Color fijo por particula (paleta HSV repartida en el circulo cromatico)
    colores = np.zeros((n_part, 4), dtype=np.ubyte)
    for k, _pid in enumerate(ids):
        c = pg.hsvColor(k / max(1, n_part), sat=0.8, val=1.0)
        colores[k] = [c.red(), c.green(), c.blue(), 220]
    color_por_id = {pid: colores[k] for k, pid in enumerate(ids)}

    # Diccionario tick -> (pos[N,2], radios[N], colores[N,4])
    cuadros = {}
    for tick, g in df.groupby("tick"):
        g = g.sort_values("id")
        pos = g[["x", "y"]].to_numpy(dtype=float)
        rad = g["radio"].to_numpy(dtype=float)
        col = np.array([color_por_id[i] for i in g["id"].to_numpy()], dtype=np.ubyte)
        vel = g[["vx", "vy"]].to_numpy(dtype=float)
        idp = g["id"].to_numpy()
        cuadros[int(tick)] = (pos, rad, col, vel, idp)

    ticks = sorted(cuadros.keys())
    print(f"[OK] {csv_path}: {n_part} particulas, {len(ticks)} ticks")
    return cuadros, ticks


# =============================================================
# VENTANA INTERACTIVA
# =============================================================
class VisorParticulas(QtWidgets.QMainWindow):
    def __init__(self, cuadros, ticks, ancho, alto, fps=30, salto=1,
                 mostrar_flechas=False, mostrar_ids=False):
        super().__init__()
        self.cuadros = cuadros
        self.ticks = ticks
        self.idx = 0                 # indice dentro de self.ticks
        self.salto = max(1, salto)
        self.mostrar_flechas = mostrar_flechas
        self.mostrar_ids = mostrar_ids
        self.textos = []
        self.reproduciendo = True

        self.setWindowTitle("Simulador V2 — Visor en tiempo real (pyqtgraph / OpenGL)")
        self.resize(900, 950)

        central = QtWidgets.QWidget()
        self.setCentralWidget(central)
        layout = QtWidgets.QVBoxLayout(central)

        # --- Area de dibujo (intenta usar OpenGL si esta disponible) ---
        try:
            pg.setConfigOptions(useOpenGL=True, antialias=True)
        except Exception:
            pass
        self.plot = pg.PlotWidget()
        self.plot.setBackground("#0d1117")
        self.plot.setAspectLocked(True)
        self.plot.setXRange(0, ancho)
        self.plot.setYRange(0, alto)
        self.plot.invertY(True)   # gravedad hacia abajo (y=0 arriba)
        self.plot.setLabel("bottom", "Posicion X (m)")
        self.plot.setLabel("left", "Posicion Y (m)")
        self.plot.showGrid(x=True, y=True, alpha=0.2)
        layout.addWidget(self.plot)

        # Un UNICO scatter reutilizable: actualizamos con setData(), no recreamos
        self.scatter = pg.ScatterPlotItem(pxMode=False, pen=pg.mkPen("k", width=0.4))
        self.plot.addItem(self.scatter)

        # Flechas de velocidad: un unico item de segmentos rojos (connect="pairs")
        self.vel_item = pg.PlotCurveItem(pen=pg.mkPen("#e74c3c", width=1.5),
                                         connect="pairs")
        self.plot.addItem(self.vel_item)
        self.vel_item.setVisible(self.mostrar_flechas)

        # --- Controles ---
        controles = QtWidgets.QHBoxLayout()

        self.btn_play = QtWidgets.QPushButton("Pausa")
        self.btn_play.clicked.connect(self.toggle_play)
        controles.addWidget(self.btn_play)

        self.btn_reset = QtWidgets.QPushButton("Reiniciar")
        self.btn_reset.clicked.connect(self.reiniciar)
        controles.addWidget(self.btn_reset)

        controles.addWidget(QtWidgets.QLabel("Velocidad (fps):"))
        self.spin_fps = QtWidgets.QSpinBox()
        self.spin_fps.setRange(1, 120)
        self.spin_fps.setValue(fps)
        self.spin_fps.valueChanged.connect(self.cambiar_fps)
        controles.addWidget(self.spin_fps)

        controles.addWidget(QtWidgets.QLabel("Salto (submuestreo):"))
        self.spin_salto = QtWidgets.QSpinBox()
        self.spin_salto.setRange(1, 50)
        self.spin_salto.setValue(self.salto)
        self.spin_salto.valueChanged.connect(self.cambiar_salto)
        controles.addWidget(self.spin_salto)

        layout.addLayout(controles)

        # Slider de tick + etiqueta
        slider_layout = QtWidgets.QHBoxLayout()
        self.slider = QtWidgets.QSlider(QtCore.Qt.Horizontal)
        self.slider.setRange(0, len(ticks) - 1)
        self.slider.valueChanged.connect(self.ir_a_indice)
        slider_layout.addWidget(self.slider)
        self.lbl_tick = QtWidgets.QLabel()
        slider_layout.addWidget(self.lbl_tick)
        layout.addLayout(slider_layout)

        # Timer de reproduccion
        self.timer = QtCore.QTimer()
        self.timer.timeout.connect(self.avanzar)
        self.timer.start(int(1000 / fps))

        self.dibujar()

    # --- Logica de dibujo ---
    def dibujar(self):
        tick = self.ticks[self.idx]
        pos, rad, col, vel, idp = self.cuadros[tick]
        # size = diametro (2*radio) en unidades del mundo, porque pxMode=False
        self.scatter.setData(x=pos[:, 0], y=pos[:, 1],
                             size=2.0 * rad, brush=col)

        # Flechas de velocidad: pares de puntos (origen -> origen + velocidad)
        if self.mostrar_flechas:
            n = len(pos)
            xs = np.empty(2 * n); ys = np.empty(2 * n)
            xs[0::2] = pos[:, 0]; xs[1::2] = pos[:, 0] + vel[:, 0]
            ys[0::2] = pos[:, 1]; ys[1::2] = pos[:, 1] + vel[:, 1]
            self.vel_item.setData(xs, ys)

        # Tamano de radio dentro de cada circulo
        if self.mostrar_ids:
            if not self.textos:
                for (xi, yi), rr in zip(pos, rad):
                    t = pg.TextItem(f"{rr:.0f}", color="w", anchor=(0.5, 0.5))
                    t.setPos(xi, yi)
                    self.plot.addItem(t)
                    self.textos.append(t)
            else:
                for t, (xi, yi) in zip(self.textos, pos):
                    t.setPos(xi, yi)

        self.lbl_tick.setText(f"Tick {tick}  ({self.idx + 1}/{len(self.ticks)})")
        # mantener slider sincronizado sin disparar bucle
        self.slider.blockSignals(True)
        self.slider.setValue(self.idx)
        self.slider.blockSignals(False)

    def avanzar(self):
        if not self.reproduciendo:
            return
        self.idx = (self.idx + self.salto) % len(self.ticks)
        self.dibujar()

    # --- Controles ---
    def toggle_play(self):
        self.reproduciendo = not self.reproduciendo
        self.btn_play.setText("Pausa" if self.reproduciendo else "Reproducir")

    def reiniciar(self):
        self.idx = 0
        self.dibujar()

    def cambiar_fps(self, v):
        self.timer.start(int(1000 / max(1, v)))

    def cambiar_salto(self, v):
        self.salto = max(1, v)

    def ir_a_indice(self, v):
        self.idx = v
        self.dibujar()

    def keyPressEvent(self, ev):
        k = ev.key()
        if k == QtCore.Qt.Key_Space:
            self.toggle_play()
        elif k == QtCore.Qt.Key_R:
            self.reiniciar()
        elif k == QtCore.Qt.Key_Right:
            self.idx = min(len(self.ticks) - 1, self.idx + 1); self.dibujar()
        elif k == QtCore.Qt.Key_Left:
            self.idx = max(0, self.idx - 1); self.dibujar()


# =============================================================
# EXPORTACION A MP4 (headless, sin abrir ventana)
# =============================================================
def exportar_mp4(cuadros, ticks, ancho, alto, salida, fps=30, salto=1):
    """
    Render headless a MP4 usando pyqtgraph + imageio/FFmpeg.
    Captura cada cuadro del scatter y lo escribe en streaming (no acumula RAM).
    """
    try:
        import imageio.v2 as imageio
    except Exception:
        print("Para exportar MP4 instala: pip install imageio imageio-ffmpeg")
        return

    app = pg.mkQApp()
    plot = pg.PlotWidget()
    plot.setBackground("#0d1117")
    plot.setAspectLocked(True)
    plot.setXRange(0, ancho)
    plot.setYRange(0, alto)
    plot.invertY(True)
    plot.resize(800, 800)
    scatter = pg.ScatterPlotItem(pxMode=False, pen=pg.mkPen("k", width=0.4))
    plot.addItem(scatter)
    exporter = pg.exporters.ImageExporter(plot.plotItem)
    exporter.params.param("width").setValue(800)

    indices = range(0, len(ticks), max(1, salto))
    print(f"Exportando {len(list(indices))} cuadros a {salida}...")
    with imageio.get_writer(salida, fps=fps, codec="libx264",
                            quality=8) as writer:
        for n, i in enumerate(range(0, len(ticks), max(1, salto))):
            pos, rad, col, vel, idp = cuadros[ticks[i]]
            scatter.setData(x=pos[:, 0], y=pos[:, 1], size=2.0 * rad, brush=col)
            app.processEvents()
            qimg = exporter.export(toBytes=True)
            w, h = qimg.width(), qimg.height()
            ptr = qimg.bits(); ptr.setsize(h * w * 4)
            arr = np.frombuffer(ptr, np.uint8).reshape(h, w, 4)[..., :3]
            writer.append_data(arr.copy())
            if (n + 1) % 25 == 0:
                print(f"  {n + 1} cuadros...", flush=True)
    print(f"Video guardado: {salida}")


# =============================================================
# PRINCIPAL
# =============================================================
def main():
    ap = argparse.ArgumentParser(description="Visor de particulas en tiempo real (pyqtgraph)")
    ap.add_argument("csv", nargs="?", default="trayectorias.csv",
                    help="Ruta a trayectorias.csv (esquema V2)")
    ap.add_argument("--ancho", type=float, default=None)
    ap.add_argument("--alto", type=float, default=None)
    ap.add_argument("--fps", type=int, default=30)
    ap.add_argument("--salto", type=int, default=1, help="submuestreo: 1 de cada k ticks")
    ap.add_argument("--exportar", metavar="salida.mp4", default=None,
                    help="Renderiza a MP4 sin abrir ventana")
    ap.add_argument("--flechas", action="store_true", help="dibuja vectores de velocidad")
    ap.add_argument("--ids", action="store_true", help="muestra el numero de cada particula")
    ap.add_argument("--simple", action="store_true", help="solo circulos (sin flechas ni ids)")
    args = ap.parse_args()

    # Estilo: por bandera, o preguntando si la terminal es interactiva
    mostrar_flechas, mostrar_ids = args.flechas, args.ids
    if not (args.flechas or args.ids or args.simple) and not args.exportar:
        try:
            print("\nEstilo de visualizacion:")
            print("  1. Simple        - circulos por radio")
            print("  2. Con flechas   - circulos + vector de velocidad")
            print("  3. Flechas + radio - circulos + flechas + tamano del radio")
            op = input("Opcion [1/2/3] (Enter = 1): ").strip()
            mostrar_flechas = op in ("2", "3")
            mostrar_ids = op == "3"
        except (EOFError, OSError):
            pass

    # localizar el CSV en rutas tipicas si no esta donde se indico
    ruta = args.csv
    if not os.path.exists(ruta):
        for d in ("target", "c_modules", "."):
            cand = os.path.join(d, os.path.basename(args.csv))
            if os.path.exists(cand):
                ruta = cand
                break

    cuadros, ticks = cargar_datos(ruta)

    # dimensiones del mundo
    todas = np.vstack([c[0] for c in cuadros.values()])
    ancho = args.ancho or float(todas[:, 0].max()) * 1.05
    alto = args.alto or float(todas[:, 1].max()) * 1.05

    if args.exportar:
        import pyqtgraph.exporters  # noqa: F401  (registra exporters)
        exportar_mp4(cuadros, ticks, ancho, alto, args.exportar,
                     fps=args.fps, salto=args.salto)
        return

    app = pg.mkQApp()
    visor = VisorParticulas(cuadros, ticks, ancho, alto, fps=args.fps, salto=args.salto,
                            mostrar_flechas=mostrar_flechas, mostrar_ids=mostrar_ids)
    visor.show()
    pg.exec()


if __name__ == "__main__":
    main()
