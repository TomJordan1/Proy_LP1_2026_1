#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
visualizar.py — Visualizador unificado del Simulador V2 (Opti.c, Grid Hashing)
Curso: SW-303 Lenguaje de Programacion I — Imperativo, Lenguaje C

Lee los CSV generados por el simulador en C (Opti.c) y produce:
  1. simulacion.gif        — Animacion de las particulas (rapida, submuestreada)
  2. grafico_energia.png   — Energia cinetica, potencial y total vs tiempo
  3. heatmap_densidad.png  — Mapa de densidad de posiciones acumuladas
  4. grafico_benchmark.png — Rendimiento (part*tick/s) y tiempo vs N particulas

IMPORTANTE — esquema de columnas de la V2 (distinto al proyecto original):
  trayectorias.csv : tick,id,x,y,vx,vy,radio
  energia.csv      : tick,cinetica,potencial,total
  benchmark.csv    : n_particulas,ticks,tiempo_segundos,part_por_segundo  (SIN cabecera)

Uso:
  python visualizar.py                         # busca CSV en ./ y ./target y ./c_modules
  python visualizar.py <ancho> <alto> <dir>    # dir donde estan los CSV
  python visualizar.py --solo-benchmark        # solo el grafico de benchmark
  python visualizar.py --sin-gif               # omite el GIF (lo pesado)

Requisitos:
  pip install pandas matplotlib numpy pillow
"""

import os
import sys

import numpy as np
import pandas as pd
import matplotlib
matplotlib.use("Agg")           # backend sin ventana: mas rapido para guardar archivos
import matplotlib.pyplot as plt

# =============================================================
# ESTILO GLOBAL — aspecto profesional
# =============================================================
plt.rcParams.update({
    "figure.figsize"   : (12, 8),
    "font.size"        : 12,
    "font.family"      : "DejaVu Sans",
    "axes.grid"        : True,
    "grid.alpha"       : 0.3,
    "grid.linestyle"   : "--",
    "axes.spines.top"  : False,
    "axes.spines.right": False,
    "figure.facecolor" : "white",
    "axes.facecolor"   : "#fafafa",
    "savefig.dpi"      : 150,
    "lines.linewidth"  : 2.0,
})

_PALETA = plt.cm.tab20.colors


# =============================================================
# AUXILIAR: localizar y cargar CSV
# =============================================================
def buscar_csv(nombre, dirs):
    """Devuelve la primera ruta existente de `nombre` entre los directorios dados."""
    for d in dirs:
        ruta = os.path.join(d, nombre)
        if os.path.exists(ruta):
            return ruta
    return None


def cargar_trayectorias(ruta):
    if ruta is None:
        print("  [OMITIDO] trayectorias.csv no encontrado.")
        return None
    df = pd.read_csv(ruta, encoding="utf-8")
    print(f"  [OK] {ruta}  ({len(df):,} filas)")
    return df


def cargar_energia(ruta):
    if ruta is None:
        print("  [OMITIDO] energia.csv no encontrado.")
        return None
    df = pd.read_csv(ruta, encoding="utf-8")
    print(f"  [OK] {ruta}  ({len(df):,} filas)")
    return df


def cargar_benchmark(ruta):
    """benchmark.csv de la V2 NO trae cabecera; se la asignamos aqui."""
    if ruta is None:
        print("  [OMITIDO] benchmark.csv no encontrado.")
        return None
    cols = ["n_particulas", "ticks", "tiempo_segundos", "part_por_segundo"]
    primera = open(ruta, encoding="utf-8").readline().lower()
    tiene_cabecera = "n_particulas" in primera or "tiempo" in primera
    df = pd.read_csv(ruta, header=0 if tiene_cabecera else None,
                     names=None if tiene_cabecera else cols, encoding="utf-8",
                     skipinitialspace=True)
    df.columns = [str(c).strip() for c in df.columns]
    # Coercer a numerico y descartar filas no validas
    for c in cols:
        df[c] = pd.to_numeric(df[c], errors="coerce")
    df = df.dropna(subset=cols)
    df = df[(df["n_particulas"] > 0) & (df["tiempo_segundos"] > 0)]
    df = df.drop_duplicates(subset="n_particulas", keep="last")
    print(f"  [OK] {ruta}  ({len(df):,} corridas)")
    return df


# =============================================================
# GRAFICO 1: ANIMACION GIF (version eficiente)
# =============================================================
def generar_animacion(df, ancho, alto, max_frames=120,
                      archivo="simulacion.gif", fps=25):
    """
    Genera el GIF de forma eficiente:
      - groupby('tick') UNA sola vez (evita filtrar el DataFrame en cada frame)
      - un unico scatter con set_offsets/set_sizes (sin Circle ni iterrows por frame)
      - submuestreo de ticks a `max_frames` cuadros como maximo

    El radio se mapea al area del marcador (s = (radio escalado)^2) para que el
    tamano visual sea proporcional al radio fisico de la particula.
    """
    if df is None:
        return

    ticks_unicos = sorted(df["tick"].unique())
    n_ticks = len(ticks_unicos)
    paso = max(1, n_ticks // max_frames)
    frames = ticks_unicos[::paso]

    grupos = {t: g for t, g in df.groupby("tick")}

    fig, ax = plt.subplots(figsize=(8, 8), dpi=90)
    fig.patch.set_facecolor("#0d1117")
    ax.set_facecolor("#0d1117")
    ax.set_xlim(0, ancho)
    ax.set_ylim(0, alto)
    ax.set_aspect("equal")
    ax.set_xlabel("Posicion X (m)", color="white")
    ax.set_ylabel("Posicion Y (m)", color="white")
    ax.tick_params(colors="white")
    for spine in ax.spines.values():
        spine.set_edgecolor("#444444")
    ax.grid(color="#333333", alpha=0.5, linestyle="--")

    pts_por_unidad = (fig.get_size_inches()[0] * 72) / ancho
    ids = sorted(df["id"].unique())
    colores = {pid: _PALETA[i % len(_PALETA)] for i, pid in enumerate(ids)}

    scatter = ax.scatter([], [], alpha=0.85, zorder=5, edgecolors="black",
                         linewidths=0.4)
    titulo = ax.set_title("", color="white", fontsize=13, fontweight="bold")

    from PIL import Image as PILImage
    frames_pil = []
    print(f"  Generando GIF eficiente ({len(frames)} cuadros de {n_ticks} ticks)...",
          flush=True)

    for k, t in enumerate(frames):
        g = grupos[t].sort_values("id")
        scatter.set_offsets(g[["x", "y"]].values)
        s = (g["radio"].values * pts_por_unidad) ** 2
        scatter.set_sizes(s)
        scatter.set_color([colores[pid] for pid in g["id"].values])
        titulo.set_text(f"Simulacion V2 (Grid Hashing) — Tick {t}")
        fig.canvas.draw()
        w_px, h_px = fig.canvas.get_width_height()
        img = PILImage.frombuffer("RGBA", (w_px, h_px),
                                  fig.canvas.buffer_rgba(), "raw", "RGBA", 0, 1)
        frames_pil.append(img.convert("P", palette=PILImage.ADAPTIVE, colors=128))
        if (k + 1) % 20 == 0 or k == len(frames) - 1:
            print(f"    {k+1}/{len(frames)} cuadros...", flush=True)

    frames_pil[0].save(archivo, save_all=True, append_images=frames_pil[1:],
                       loop=0, duration=int(1000 / fps), optimize=False)
    plt.close(fig)
    print(f"Grafico guardado: {archivo}")


# =============================================================
# GRAFICO 2: ENERGIA vs TIEMPO
# =============================================================
def graficar_energia(df, dt=0.008, archivo="grafico_energia.png"):
    if df is None:
        return
    tiempo = df["tick"] * dt

    fig, ax = plt.subplots(figsize=(13, 6))
    ax.plot(tiempo, df["cinetica"], color="#e74c3c", label="Energia Cinetica Ek")
    ax.plot(tiempo, df["potencial"], color="#2980b9", label="Energia Potencial Ep")
    ax.plot(tiempo, df["total"], color="#27ae60", linewidth=2.5, linestyle="--",
            label="Energia Total Et = Ek + Ep")

    et_ini = df["total"].iloc[0]
    ax.axhline(et_ini, color="#27ae60", linestyle=":", alpha=0.4, linewidth=1.5,
               label=f"Et inicial = {et_ini:.0f} J")

    variacion = abs(df["total"].max() - df["total"].min())
    t_mid = tiempo.iloc[len(tiempo) // 2]
    ax.annotate(f"Variacion Et: {variacion:.0f} J\n(error Euler + friccion + rebotes)",
                xy=(t_mid, (df["total"].max() + df["total"].min()) / 2),
                fontsize=10, color="#555555", ha="center",
                bbox=dict(boxstyle="round,pad=0.4", facecolor="#fffbe6",
                          edgecolor="#cccc00", alpha=0.85))

    ax.set_xlabel("Tiempo (s)")
    ax.set_ylabel("Energia (J)")
    ax.set_title("Evolucion de Energias del Sistema (Simulador V2)",
                 fontsize=15, fontweight="bold", pad=12)
    ax.legend(fontsize=11, loc="upper right", framealpha=0.9, edgecolor="#dddddd")
    plt.tight_layout()
    plt.savefig(archivo)
    plt.close(fig)
    print(f"Grafico guardado: {archivo}")


# =============================================================
# GRAFICO 3: HEATMAP DE DENSIDAD
# =============================================================
def graficar_heatmap(df, ancho, alto, archivo="heatmap_densidad.png"):
    if df is None:
        return
    fig, ax = plt.subplots(figsize=(10, 8))
    conteos, xedges, yedges = np.histogram2d(
        df["x"].values, df["y"].values, bins=70,
        range=[[0, ancho], [0, alto]])
    xc = 0.5 * (xedges[:-1] + xedges[1:])
    yc = 0.5 * (yedges[:-1] + yedges[1:])
    X, Y = np.meshgrid(xc, yc)
    mapa = ax.pcolormesh(X, Y, conteos.T, cmap="inferno", shading="auto")
    barra = plt.colorbar(mapa, ax=ax, pad=0.02)
    barra.set_label("Visitas acumuladas (ticks)")
    ax.set_xlabel("Posicion X (m)")
    ax.set_ylabel("Posicion Y (m)")
    ax.set_title("Mapa de Densidad de Posiciones\n(todas las particulas, todos los ticks)",
                 fontsize=13, fontweight="bold", pad=12)
    ax.set_xlim(0, ancho)
    ax.set_ylim(0, alto)
    ax.set_aspect("equal")
    plt.tight_layout()
    plt.savefig(archivo)
    plt.close(fig)
    print(f"Grafico guardado: {archivo}")


# =============================================================
# GRAFICO 4: BENCHMARK
# =============================================================
def graficar_benchmark(df, archivo="grafico_benchmark.png"):
    if df is None or len(df) == 0:
        return
    df = df.sort_values("n_particulas").reset_index(drop=True)
    n = df["n_particulas"].values.astype(float)
    t = df["tiempo_segundos"].values
    r = df["part_por_segundo"].values / 1e6

    fig, axes = plt.subplots(1, 2, figsize=(15, 6))
    fig.suptitle("Analisis de Rendimiento — Simulador V2 (Grid Hashing O(N))",
                 fontsize=15, fontweight="bold", y=1.02)

    ax1 = axes[0]
    ax1.plot(n, t, "o-", color="#e74c3c", markersize=9, label="Tiempo medido (s)", zorder=5)
    if len(n) >= 2 and n[0] > 0:
        esc_lin = t[0] / n[0]
        ax1.plot(n, esc_lin * n, "--", color="#3498db", alpha=0.8, label="Referencia O(N)")
        esc_cuad = t[0] / (n[0] ** 2)
        ax1.plot(n, esc_cuad * n ** 2, ":", color="#95a5a6", alpha=0.8,
                 label="Referencia O(N^2)")
    ax1.set_xlabel("N particulas")
    ax1.set_ylabel("Tiempo de ejecucion (s)")
    ax1.set_title("Tiempo total vs N particulas")
    ax1.legend(fontsize=10)
    for xi, ti in zip(n, t):
        ax1.annotate(f"{ti:.3f}s", xy=(xi, ti), xytext=(0, 8),
                     textcoords="offset points", ha="center", fontsize=9)

    ax2 = axes[1]
    cols = [_PALETA[i % len(_PALETA)] for i in range(len(n))]
    barras = ax2.bar([str(int(v)) for v in n], r, color=cols, alpha=0.85,
                     edgecolor="white", linewidth=0.8)
    for b, rv in zip(barras, r):
        ax2.text(b.get_x() + b.get_width() / 2, b.get_height() + max(r) * 0.01,
                 f"{rv:.2f}M", ha="center", va="bottom", fontsize=9, fontweight="bold")
    ax2.set_xlabel("N particulas")
    ax2.set_ylabel("Tasa (millones de part*tick / s)")
    ax2.set_title("Tasa de Procesamiento vs N")
    ax2.grid(True, alpha=0.3, axis="y")
    ax2.set_ylim(0, max(r) * 1.15)

    plt.tight_layout()
    plt.savefig(archivo)
    plt.close(fig)
    print(f"Grafico guardado: {archivo}")


# =============================================================
# PRINCIPAL
# =============================================================
def main():
    args = [a for a in sys.argv[1:] if not a.startswith("--")]
    solo_bench = "--solo-benchmark" in sys.argv
    sin_gif = "--sin-gif" in sys.argv

    dirs = [".", "target", "c_modules"]
    ancho = alto = None
    if len(args) >= 3:
        ancho, alto = float(args[0]), float(args[1])
        dirs = [args[2]] + dirs

    print("=" * 56)
    print("  Visualizador Simulador V2 — SW-303 (Grid Hashing)")
    print("=" * 56)

    if solo_bench:
        graficar_benchmark(cargar_benchmark(buscar_csv("benchmark.csv", dirs)))
        return

    print("\nCargando CSV...")
    df_tray = cargar_trayectorias(buscar_csv("trayectorias.csv", dirs))
    df_ener = cargar_energia(buscar_csv("energia.csv", dirs))
    df_bench = cargar_benchmark(buscar_csv("benchmark.csv", dirs))

    if df_tray is not None:
        if ancho is None:
            ancho = float(df_tray["x"].max()) * 1.05
            alto = float(df_tray["y"].max()) * 1.05
        print(f"\n  Espacio   : {ancho:.0f} x {alto:.0f} m")
        print(f"  Particulas: {df_tray['id'].nunique()}")
        print(f"  Ticks     : {df_tray['tick'].nunique()}\n")
    else:
        ancho, alto = ancho or 1000.0, alto or 1000.0

    if not sin_gif:
        print("[1/4] Animacion GIF...")
        generar_animacion(df_tray, ancho, alto, max_frames=120,
                          archivo="simulacion.gif")
    else:
        print("[1/4] Animacion GIF... OMITIDA (--sin-gif)")

    print("\n[2/4] Grafico de energias...")
    graficar_energia(df_ener)
    print("\n[3/4] Heatmap de densidad...")
    graficar_heatmap(df_tray, ancho, alto)
    print("\n[4/4] Grafico de benchmark...")
    graficar_benchmark(df_bench)

    print("\n" + "=" * 56)
    print("  Listo. Archivos generados:")
    for f in ["simulacion.gif", "grafico_energia.png",
              "heatmap_densidad.png", "grafico_benchmark.png"]:
        estado = "OK" if os.path.exists(f) else "--"
        print(f"    [{estado}] {f}")
    print("=" * 56)


if __name__ == "__main__":
    main()
           