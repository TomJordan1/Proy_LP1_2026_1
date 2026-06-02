#include "Simulador.h"

FILE *abrir_csv_trayectorias() {
    FILE *f = fopen("trayectorias.csv", "w");
    if (!f) {
        fprintf(stderr,
                "Error: No se pudo crear trayectorias.csv.\n"
                "Verificar permisos de escritura en el directorio.\n");
        exit(EXIT_FAILURE);
    }
    fprintf(f, "tick,id,x,y,vx,vy,radio\n");
    return f;
}

FILE *abrir_csv_energia() {
    FILE *f = fopen("energia.csv", "w");
    if (!f) {
        fprintf(stderr,
                "Error: No se pudo crear energia.csv.\n"
                "Verificar permisos de escritura en el directorio.\n");
        exit(EXIT_FAILURE);
    }
    fprintf(f, "tick,cinetica,potencial,total\n");
    return f;
}

void exportar_tick_trayectoria(FILE *f, int t, int i, Particula p) {
    fprintf(f, "%d,%d,%.2f,%.2f,%.2f,%.2f,%.2f\n", t, i, p.x, p.y, p.vx, p.vy, p.r);
}

void exportar_tick_energia(FILE *f, int tick, double ec, double ep) {
    fprintf(f, "%d,%.2f,%.2f,%.2f\n", tick, ec, ep, ec + ep);
}

void exportar_benchmark(int n, int ticks, double tiempo) {
    FILE *f = fopen("benchmark.csv", "a");
    if (!f) {
        fprintf(stderr, "Error: No se pudo abrir 'benchmark.csv' para escritura.\n");
        return;
    }
    double tasa = (float)n * ticks / tiempo;
    fprintf(f, "%d,%d,%.4f,%.4f\n", n, ticks, tiempo, tasa);
    fclose(f);
}

void exportar_dimensiones_caja(Config c) {
    FILE *f = fopen("dimensiones.csv", "w");
    if (!f) {
        fprintf(stderr, 
                "Error: No se pudo crear dimensiones.csv.\n"
                "Verificar permisos de escritura en el directorio.\n");
        exit(EXIT_FAILURE);
    }
    fprintf(f, "ancho,alto\n");
    fprintf(f, "%.2f,%.2f\n", c.ancho, c.alto);
    fclose(f);
}