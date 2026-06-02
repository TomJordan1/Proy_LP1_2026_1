#ifndef FISICA
#define FISICA
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct {
    double x, y;
    double vx, vy;
    double m;
    double r;
} Particula;

typedef struct {
    int n_particulas;
    int ticks;
    double dt;
    double g;
    double e;
    double mu;
    double ancho;
    double alto;
} Config;
void actualizar_fisica_basica(Particula *p, Config c);
void resolver_colision_paredes(Particula *p, Config c);
void resolver_colision_particulas(Particula *p1, Particula *p2);
Config configurar_sistema();

FILE *abrir_csv_trayectorias();
FILE *abrir_csv_energia();
void exportar_tick_trayectoria(FILE *f, int t, int i, Particula p);
void exportar_tick_energia(FILE *f, int tick, double ec, double ep);
void exportar_benchmark(int n, int ticks, double tiempo);
#endif