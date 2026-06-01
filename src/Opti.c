#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define MX 500005
const unsigned int p1 = 73856093, p2 = 19349663;

//  ESTRUCTURAS DE DATOS
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

Particula p[MX];
int head[12 * MX], next[MX];

// FUNCIONES DE EVENTOS FÍSICOS

void actualizar_fisica_basica(Particula *p, Config c) {
    p->vy += c.g * c.dt;
    p->vx *= (1.0 - c.mu * c.dt);
    p->vy *= (1.0 - c.mu * c.dt);
    p->x += p->vx * c.dt;
    p->y += p->vy * c.dt;
}

void resolver_colision_paredes(Particula *p, Config c) {
    if (p->x < p->r) {
        p->x = p->r;
        p->vx *= -c.e;
    }
    if (p->x > c.ancho - p->r) {
        p->x = c.ancho - p->r;
        p->vx *= -c.e;
    }
    if (p->y < p->r) {
        p->y = p->r;
        p->vy *= -c.e;
    }
    if (p->y > c.alto - p->r) {
        p->y = c.alto - p->r;
        p->vy *= -c.e;
    }
}

void resolver_colision_particulas(Particula *p1, Particula *p2) {
    double dx = p2->x - p1->x;
    double dy = p2->y - p1->y;
    double dist = sqrt(dx * dx + dy * dy);

    if (dist < p1->r + p2->r && dist > 0.0001) {
        double solapamiento = p1->r + p2->r - dist;
        double nx = dx / dist;
        double ny = dy / dist;

        double masa_total = p1->m + p2->m;
        double m1 = p2->m / masa_total;
        double m2 = p1->m / masa_total;

        p1->x -= solapamiento * nx * m2;
        p1->y -= solapamiento * ny * m2;
        p2->x += solapamiento * nx * m1;
        p2->y += solapamiento * ny * m1;

        double dvx = p2->vx - p1->vx;
        double dvy = p2->vy - p1->vy;
        double dot_product = dvx * nx + dvy * ny;

        if (dot_product < 0) {
            double e = 1.0;
            double impulso = -(1.0 + e) * dot_product;
            impulso /= (1.0 / p1->m + 1.0 / p2->m);

            double ix = impulso * nx;
            double iy = impulso * ny;

            p1->vx -= ix / p1->m;
            p1->vy -= iy / p1->m;
            p2->vx += ix / p2->m;
            p2->vy += iy / p2->m;
        }
    }
}

// FUNCIÓN DE HASHING ESPACIAL

unsigned int hash(int cx, int cy, int hash_size) {
    /* XOR bit a bit de coordenadas multiplicadas por primos, mapeado al tamaño de la tabla */
    return (unsigned int)((cx * p1) ^ (cy * p2)) % hash_size;
}

// FUNCIONES DE INTERFAZ / CONFIGURACIÓN

Config configurar_sistema() {
    Config c;
    char opcion;

    printf("--- SIMULADOR CON GRID HASHING ESPACIAL $O(N)$ ---\n");
    printf("Dimensiones del entorno (Ancho): ");
    scanf("%lf", &c.ancho);
    printf("Dimensiones del entorno (Alto): ");
    scanf("%lf", &c.alto);
    printf("Numero de particulas: ");
    scanf("%d", &c.n_particulas);
    getchar();

    printf("¿Usar parametros fisicos predeterminados? (s/n): ");
    scanf(" %c", &opcion);
    getchar();

    if (opcion == 's' || opcion == 'S') {
        c.ticks = 10;
        c.dt = 0.008;
        c.g = 9.81;
        c.e = 0.85;
        c.mu = 0.01;
    } else {
        printf("Ticks totales: ");
        scanf("%d", &c.ticks);
        printf("dt (paso tiempo): ");
        scanf("%lf", &c.dt);
        printf("Gravedad: ");
        scanf("%lf", &c.g);
        printf("Restitucion paredes (0-1): ");
        scanf("%lf", &c.e);
        printf("Rozamiento: ");
        scanf("%lf", &c.mu);
        getchar();
    }
    return c;
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

    /* 4. Datos del Spatial Hashing (Limitados a 12 * N) */
    double lado_celda = 2.0 * max_r;
    int hash_size = 12 * conf.n_particulas; /* Memoria O(N) */

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
        /* BUCLE B: SPATIAL HASHING O(N) EN MEMORIA Y PROCESO */
        /* B.1 - Vaciar la tabla Hash (solo toma 4N iteraciones) */
        memset(head, -1, hash_size * sizeof(int));
        /* B.2 - Llenar la tabla Hash */
        for (int i = 0; i < conf.n_particulas; i++) {
            int cx = (int)(p[i].x / lado_celda);
            int cy = (int)(p[i].y / lado_celda);

            unsigned int h = hash(cx, cy, hash_size);

            next[i] = head[h];
            head[h] = i;
        }

        /* B.3 - Verificar colisiones (Recorriendo partículas, no el espacio vacío) */
        for (int i = 0; i < conf.n_particulas; i++) {
            int cx = (int)(p[i].x / lado_celda);
            int cy = (int)(p[i].y / lado_celda);

            /* a) Choques dentro de la MISMA celda (se usa j > i para no repetir) */
            unsigned int h = hash(cx, cy, hash_size);
            int j = head[h];
            while (j != -1) {
                if (j > i) {
                    /* Verificar colisión real de celda (evitar falsos positivos del Hash) */
                    int cjx = (int)(p[j].x / lado_celda);
                    int cjy = (int)(p[j].y / lado_celda);
                    if (cx == cjx && cy == cjy) {
                        resolver_colision_particulas(&p[i], &p[j]);
                    }
                }
                j = next[j];
            }

            /* b) Choques con celdas VECINAS (4 direcciones) */
            int dx[4] = {1, -1, 0, 1};
            int dy[4] = {0, 1, 1, 1};
            for (int o = 0; o < 4; o++) {
                int nx = cx + dx[o];
                int ny = cy + dy[o];

                unsigned int h_vecina = hash(nx, ny, hash_size);
                int nj = head[h_vecina];

                while (nj != -1) {
                    /* Verificar que pertenecen a la coordenada vecina exacta */
                    int cnjx = (int)(p[nj].x / lado_celda);
                    int cnjy = (int)(p[nj].y / lado_celda);
                    if (nx == cnjx && ny == cnjy) {
                        resolver_colision_particulas(&p[i], &p[nj]);
                    }
                    nj = next[nj];
                }
            }
        }
        /* ---------------------------------------------------- */

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
    fprintf(f_bench, "%d,%d,%.4f\n", conf.n_particulas, conf.ticks, total_t);

    fclose(f_tray);
    fclose(f_ener);
    fclose(f_bench);

    printf("\nSimulacion completada en %.4fs.\n", total_t);
    return 0;
}