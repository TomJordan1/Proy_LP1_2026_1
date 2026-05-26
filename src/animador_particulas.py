# -*- coding: utf-8 -*-
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
    
    def update(tick):
        ax.clear()
        
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
    print("--- ANIMADOR DE COLISIONES AUTOMATIZADO ---")
    
    # Leer argumentos desde la terminal (provistos por el Makefile)
    if len(sys.argv) >= 4:
        ancho = float(sys.argv[1])
        alto = float(sys.argv[2])
        archivo_csv = sys.argv[3]
    else:
        # Fallback interactivo si se ejecuta manualmente
        print("Argumentos insuficientes, modo manual...")
        archivo_csv = 'target/trayectorias.csv'
        ancho = float(input("Ingresa el Ancho del entorno: "))
        alto = float(input("Ingresa el Alto del entorno: "))

    MI_CAJA = (0, ancho, 0, alto)
    
    # Asegurar que el GIF se guarde en la misma carpeta que el CSV
    directorio = os.path.dirname(archivo_csv)
    if directorio == "":
        directorio = "."
    output_path = os.path.join(directorio, 'simulacion_colisiones.gif')
    
    animar_particulas(archivo_csv, limites_caja=MI_CAJA, output_gif=output_path, fps=30)
