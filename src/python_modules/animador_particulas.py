# -*- coding: utf-8 -*-
"""
Script para animar trayectorias.csv generado desde Opti.c.
"""

import os
import sys
import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from matplotlib.patches import Circle

def animar_particulas(csv_path, limites_caja, output_gif='simulacion_colisiones.gif', fps=30):
    if not os.path.exists(csv_path):
        print(f"Error: No se encontró el archivo '{csv_path}'.")
        sys.exit(1)
        
    print(f"Cargando datos desde {csv_path}...")
    df = pd.read_csv(csv_path)
    
    df = df.sort_values(by=['tick', 'id'])
    ticks = sorted(df['tick'].unique())
    
    x_min, x_max, y_min, y_max = limites_caja
    
    fig, ax = plt.subplots(figsize=(8, 8))
    
    print(f"Renderizando {len(ticks)} fotogramas...")
    
    def update(tick):
        ax.clear()
        
        # Ajustamos los límites a lo ingresado por consola
        ax.set_xlim(x_min, x_max)
        ax.set_ylim(y_min, y_max)
        ax.set_aspect('equal') 
        
        ax.grid(True, linestyle=':', alpha=0.5, color='gray')
        ax.set_facecolor('#f0f4f8')
        
        for spine in ax.spines.values():
            spine.set_linewidth(2)
            spine.set_color('#2c3e50')
            
        ax.set_title(f"Simulación de Colisiones (Opti.c)\nTick: {tick}", fontsize=12, fontweight='bold', pad=10)
        
        df_tick = df[df['tick'] == tick]
        
        for _, row in df_tick.iterrows():
            circulo = Circle(
                xy=(row['x'], row['y']), 
                radius=row['radio'],
                edgecolor='black', 
                facecolor='#2980b9', 
                alpha=0.8, 
                lw=1.2,
                zorder=3
            )
            ax.add_patch(circulo)
            
            if abs(row['vx']) > 0.01 or abs(row['vy']) > 0.01:
                ax.quiver(
                    row['x'], row['y'], row['vx'], row['vy'],
                    angles='xy', scale_units='xy', scale=1,
                    color='#c0392b', width=0.003, headwidth=4, zorder=4
                )
            
            ax.text(
                row['x'], row['y'], str(int(row['id'])),
                ha='center', va='center', fontsize=7, color='white', 
                fontweight='bold', zorder=5
            )
            
        return []

    ani = animation.FuncAnimation(fig, update, frames=ticks, blit=False)
    
    print(f"Guardando {output_gif}...")
    ani.save(output_gif, writer='pillow', fps=fps)
    plt.close(fig)
    print("¡Animación guardada con éxito!")

if __name__ == '__main__':
    archivo_csv = 'trayectorias.csv'
    
    print("--- ANIMADOR DE COLISIONES ---")
    
    # Intentar deducir la caja para dar una sugerencia (basado en (x,y) máximos del CSV)
    try:
        if os.path.exists(archivo_csv):
            df_temp = pd.read_csv(archivo_csv)
            max_x = (df_temp['x'] + df_temp['radio']).max()
            max_y = (df_temp['y'] + df_temp['radio']).max()
            print(f"(Basado en el CSV, parece que tu caja es de al menos Ancho ~ {max_x:.0f}, Alto ~ {max_y:.0f})")
    except:
        pass

    # Preguntarle al usuario igual que en C
    try:
        ancho = float(input("Ingresa el Ancho del entorno que usaste en C: "))
        alto = float(input("Ingresa el Alto del entorno que usaste en C: "))
    except ValueError:
        print("Entrada inválida. Se usarán 1000x1000 por defecto.")
        ancho, alto = 1000, 1000
        
    MI_CAJA = (0, ancho, 0, alto) 
    
    animar_particulas(archivo_csv, limites_caja=MI_CAJA, output_gif='simulacion_colisiones.gif', fps=30)