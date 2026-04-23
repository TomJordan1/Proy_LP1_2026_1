/* =============================================================
 * SIMULADOR DE N PARTÍCULAS EN 2D — SW-303 Lenguaje C
 * Archivo  : csv_export.c
 * Propósito: Funciones de exportación de datos a archivos CSV
 * Estándar : C99 / C11
 *
 * Archivos generados:
 *   trayectorias.csv  — posición y velocidad por tick y partícula
 *   energia.csv       — energías del sistema por tick
 *   benchmark.csv     — rendimiento de la simulación (modo append)
 * ============================================================= */
#include <stdio.h>
#include <stdlib.h>
#include "particula.h"

/* ---------------------------------------------------------------
 * abrir_csv_trayectorias()
 * Crea (o sobreescribe) el archivo de trayectorias y escribe el header.
 * Columnas: tick, particula_id, x, y, vx, vy
 *
 * Termina el programa con EXIT_FAILURE si fopen retorna NULL.
 * --------------------------------------------------------------- */
FILE* abrir_csv_trayectorias(const char* nombre) {
    FILE* f = fopen(nombre, "w");
    if (!f) {
        fprintf(stderr, "Error: No se pudo crear '%s'.\n"
                        "Verificar permisos de escritura en el directorio.\n",
                nombre);
        exit(EXIT_FAILURE);
    }
    /* Header CSV — encoding UTF-8 */
    fprintf(f, "tick,particula_id,x,y,vx,vy\n");
    return f;
}

/* ---------------------------------------------------------------
 * abrir_csv_energia()
 * Crea (o sobreescribe) el archivo de energías y escribe el header.
 * Columnas: tick, energia_cinetica, energia_potencial, energia_total
 * --------------------------------------------------------------- */
FILE* abrir_csv_energia(const char* nombre) {
    FILE* f = fopen(nombre, "w");
    if (!f) {
        fprintf(stderr, "Error: No se pudo crear '%s'.\n"
                        "Verificar permisos de escritura en el directorio.\n",
                nombre);
        exit(EXIT_FAILURE);
    }
    fprintf(f, "tick,energia_cinetica,energia_potencial,energia_total\n");
    return f;
}

/* ---------------------------------------------------------------
 * exportar_tick_trayectorias()
 * Escribe una fila por cada partícula en el tick indicado.
 *
 * Formato de fila:
 *   <tick>,<id>,<x:.4f>,<y:.4f>,<vx:.4f>,<vy:.4f>
 *
 * 4 decimales: precisión adecuada sin generar archivos excesivamente grandes.
 * Con 50 partículas × 500 ticks → 25,000 filas ≈ 1.5 MB.
 * --------------------------------------------------------------- */
void exportar_tick_trayectorias(FILE* f, int tick, Particula* p, int n) {
    int i;
    for (i = 0; i < n; i++) {
        fprintf(f, "%d,%d,%.4f,%.4f,%.4f,%.4f\n",
                tick,
                i,
                p[i].x,
                p[i].y,
                p[i].vx,
                p[i].vy);
    }
}

/* ---------------------------------------------------------------
 * exportar_tick_energia()
 * Escribe una fila con las tres magnitudes de energía del sistema.
 *
 * Formato de fila:
 *   <tick>,<Ek:.6f>,<Ep:.6f>,<Et:.6f>
 *
 * 6 decimales para detectar variaciones pequeñas en Et (error numérico).
 * --------------------------------------------------------------- */
void exportar_tick_energia(FILE* f, int tick, double ek, double ep, double et) {
    fprintf(f, "%d,%.6f,%.6f,%.6f\n", tick, ek, ep, et);
}

/* ---------------------------------------------------------------
 * exportar_benchmark()
 * Registra el rendimiento de la simulación en modo append.
 *
 * Si el archivo no existe, crea el header automáticamente.
 * Si ya existe (de una ejecución anterior), agrega solo la fila nueva.
 * Esto permite acumular múltiples corridas con distintos N para
 * generar el gráfico de benchmark comparativo.
 *
 * Columnas: n_particulas, ticks, tiempo_segundos, particulas_por_segundo
 *
 * Tasa = (n_particulas × ticks) / tiempo_segundos
 *   Interpretación: cuántos pares "partícula·tick" procesa el sistema por segundo.
 * --------------------------------------------------------------- */
void exportar_benchmark(const char* nombre, int n, int ticks, double tiempo) {
    /* Verificar si el archivo ya existe para no repetir el header */
    FILE* prueba = fopen(nombre, "r");
    int   existe = (prueba != NULL);
    if (prueba) {
        fclose(prueba);
    }

    /* Abrir en modo append para acumular múltiples corridas */
    FILE* f = fopen(nombre, "a");
    if (!f) {
        fprintf(stderr, "Error: No se pudo abrir '%s' para escritura.\n", nombre);
        return;  /* No terminar el programa por fallo de benchmark */
    }

    /* Escribir header solo si el archivo era nuevo */
    if (!existe) {
        fprintf(f, "n_particulas,ticks,tiempo_segundos,particulas_por_segundo\n");
    }

    /* Calcular tasa de procesamiento: evitar división por cero */
    double tasa = (tiempo > 1e-9)
                  ? ((double)n * (double)ticks) / tiempo
                  : 0.0;

    fprintf(f, "%d,%d,%.6f,%.2f\n", n, ticks, tiempo, tasa);
    fclose(f);

    printf("Archivo CSV generado: %s\n", nombre);
}
