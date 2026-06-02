#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "Simulador.h"

#define MX 500005
const unsigned int p1 = 73856093, p2 = 19349663;

Particula p[MX];
int head[12 * MX], next[MX];

static unsigned int hash(int cx, int cy, int hash_size) {
    
    return (unsigned int)((cx * p1) ^ (cy * p2)) % hash_size;
}

int main() {
    srand(time(NULL));

    Config conf = configurar_sistema();

    int modo_init;
    printf("\n1. Aleatorio\n2. Manual\nOpcion de inicializacion: ");
    scanf("%d", &modo_init);
    double max_r = 0.0;

    for (int i = 0; i < conf.n_particulas; i++) {
        if (modo_init == 1) {
            p[i].r = 5.0 + ((double)rand() / RAND_MAX) * 15.0;
            p[i].m = p[i].r * 0.5;
            p[i].x = p[i].r + ((double)rand() / RAND_MAX) * (conf.ancho - 2 * p[i].r);
            p[i].y = p[i].r + ((double)rand() / RAND_MAX) * (conf.alto - 2 * p[i].r);
            p[i].vx = ((double)rand() / RAND_MAX) * 100.0 - 50.0;
            p[i].vy = ((double)rand() / RAND_MAX) * 100.0 - 50.0;
        } else {
            printf("\nParticula %d:\n", i);
            printf("Radio y Masa: ");
            scanf("%lf %lf", &p[i].r, &p[i].m);
            printf("x, y, vx, vy: ");
            scanf("%lf %lf %lf %lf", &p[i].x, &p[i].y, &p[i].vx, &p[i].vy);
        }
        if (p[i].r > max_r) max_r = p[i].r;
    }

    
    double lado_celda = 2.0 * max_r;
    int hash_size = 12 * conf.n_particulas; 

    FILE *f_tray = fopen("trayectorias.csv", "w");
    FILE *f_ener = fopen("energia.csv", "w");
    fprintf(f_tray, "tick,id,x,y,vx,vy,radio\n");
    fprintf(f_ener, "tick,cinetica,potencial,total\n");

    clock_t inicio = clock();

    for (int t = 0; t < conf.ticks; t++) {
        for (int i = 0; i < conf.n_particulas; i++) {
            actualizar_fisica_basica(&p[i], conf);
            resolver_colision_paredes(&p[i], conf);
        }
        
        memset(head, -1, hash_size * sizeof(int));
       
        for (int i = 0; i < conf.n_particulas; i++) {
            int cx = (int)(p[i].x / lado_celda);
            int cy = (int)(p[i].y / lado_celda);

            unsigned int h = hash(cx, cy, hash_size);

            next[i] = head[h];
            head[h] = i;
        }

    
        for (int i = 0; i < conf.n_particulas; i++) {
            int cx = (int)(p[i].x / lado_celda);
            int cy = (int)(p[i].y / lado_celda);

           
            unsigned int h = hash(cx, cy, hash_size);
            int j = head[h];
            while (j != -1) {
                if (j > i) {
                    
                    int cjx = (int)(p[j].x / lado_celda);
                    int cjy = (int)(p[j].y / lado_celda);
                    if (cx == cjx && cy == cjy) {
                        resolver_colision_particulas(&p[i], &p[j]);
                    }
                }
                j = next[j];
            }

            
            int dx[4] = {1, -1, 0, 1};
            int dy[4] = {0, 1, 1, 1};
            for (int o = 0; o < 4; o++) {
                int nx = cx + dx[o];
                int ny = cy + dy[o];

                unsigned int h_vecina = hash(nx, ny, hash_size);
                int nj = head[h_vecina];

                while (nj != -1) {
                    
                    int cnjx = (int)(p[nj].x / lado_celda);
                    int cnjy = (int)(p[nj].y / lado_celda);
                    if (nx == cnjx && ny == cnjy) {
                        resolver_colision_particulas(&p[i], &p[nj]);
                    }
                    nj = next[nj];
                }
            }
        }

        double ec = 0, ep = 0;
        for (int i = 0; i < conf.n_particulas; i++) {
            ec += 0.5 * p[i].m * (p[i].vx * p[i].vx + p[i].vy * p[i].vy);
            ep += p[i].m * conf.g * (conf.alto - p[i].y);

            fprintf(f_tray, "%d,%d,%.2f,%.2f,%.2f,%.2f,%.2f\n", t, i, p[i].x, p[i].y, p[i].vx,
                    p[i].vy, p[i].r);
        }
        fprintf(f_ener, "%d,%.2f,%.2f,%.2f\n", t, ec, ep, ec + ep);
    }

    double total_t = (double)(clock() - inicio) / CLOCKS_PER_SEC;
    FILE *f_bench = fopen("benchmark.csv", "a");
    fprintf(f_bench, "%d,%d,%.4f, %.4f\n", conf.n_particulas, conf.ticks, total_t,
            (float)conf.n_particulas * conf.ticks / total_t);

    fclose(f_tray);
    fclose(f_ener);
    fclose(f_bench);

    printf("\nSimulacion completada en %.4fs.\n", total_t);
    return 0;
}