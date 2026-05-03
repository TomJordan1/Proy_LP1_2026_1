#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ==============================
 * ESTRUCTURAS DE DATOS
 * ============================== */

typedef struct {
    double x, y;   // Posición actual de la partícula
    double vx, vy; // Velocidad actual de la partícula
    double m;      // Masa de la partícula
    double r;      // Radio de la partícula
} Particula;

typedef struct {
    int n_particulas; // Número total de partículas en la simulación
    int ticks;        // Número total de iteraciones (ticks) a simular
    double dt;        // Paso de tiempo para cada tick
    double g;         // Aceleración debida a la gravedad
    double e;         // Coeficiente de restitución para colisiones con paredes (0-1)
    double mu;        // Coeficiente de rozamiento (0-1)
    double ancho;     // Ancho del entorno de simulación
    double alto;      // Alto del entorno de simulación
} Config;

/* ==========================================
 * FUNCIONES PURAS DE EVENTOS FÍSICOS
 * ========================================== */

/* 1. Actualiza velocidad y posición (Euler Semi-explícito con rozamiento) */
void actualizar_fisica_basica(Particula *p, Config c) {
    /* Gravedad */
    p->vy += c.g * c.dt;

    /* Rozamiento */
    p->vx *= (1.0 - c.mu * c.dt);
    p->vy *= (1.0 - c.mu * c.dt);

    /* Posición */
    p->x += p->vx * c.dt;
    p->y += p->vy * c.dt;
}

/* 2. Verifica y resuelve el rebote con los bordes del rectángulo */
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

/* 3. Verifica y resuelve la colisión elástica entre dos partículas */
void resolver_colision_particulas(Particula *p1, Particula *p2) {
    double dx = p2->x - p1->x;
    double dy = p2->y - p1->y;
    double dist = sqrt(dx * dx + dy * dy);

    if (dist < p1->r + p2->r) {
        /* Separar partículas basándose en la proporción de sus masas */
        double solapamiento = p1->r + p2->r - dist;
        double nx = dx / dist;
        double ny = dy / dist;

        double masa_total = p1->m + p2->m;
        double ratio1 = p2->m / masa_total;
        double ratio2 = p1->m / masa_total;

        p1->x -= solapamiento * nx * ratio1;
        p1->y -= solapamiento * ny * ratio1;
        p2->x += solapamiento * nx * ratio2;
        p2->y += solapamiento * ny * ratio2;

        /* Intercambio de impulso para masas diferentes */
        double dvx = p2->vx - p1->vx;
        double dvy = p2->vy - p1->vy;
        double dot_product = dvx * nx + dvy * ny;

        if (dot_product < 0) {
            /* Coeficiente de restitución entre partículas (asumimos 1.0 = elástico ideal) */
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

/* ==========================================
 * FUNCIONES DE INTERFAZ / CONFIGURACIÓN
 * ========================================== */

Config configurar_sistema() {
    Config c;
    char opcion;

    printf("--- SIMULADOR MODULAR (MASAS Y RADIOS VARIABLES) ---\n");

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
        c.ticks = 1000;
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

    /* 1. Configuración general */
    Config conf = configurar_sistema();

    int modo_init;
    printf("\n1. Aleatorio\n2. Manual\nOpcion de inicializacion: ");
    scanf("%d", &modo_init);

    /* 2. Crear array de partículas */
    Particula p[conf.n_particulas];

    /* 3. Inicializar las partículas (ahora incluyendo Masa y Radio) */
    for (int i = 0; i < conf.n_particulas; i++) {
        if (modo_init == 1) {
            p[i].r = 5.0 + ((double)rand() / RAND_MAX) * 15.0; /* Radio entre 5 y 20 */
            p[i].m = p[i].r * 0.5;                             /* Masa proporcional al radio */
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
    }

    /* 4. Abrir archivos CSV */
    FILE *f_tray = fopen("trayectorias.csv", "w");
    FILE *f_ener = fopen("energia.csv", "w");
    fprintf(f_tray, "tick,id,x,y,vx,vy,radio\n");
    fprintf(f_ener, "tick,cinetica,potencial,total\n");

    clock_t inicio = clock();

    /* ==========================================
     * 5. BUCLE PRINCIPAL DE SIMULACIÓN EN MAIN
     * ========================================== */
    for (int t = 0; t < conf.ticks; t++) {
        /* BUCLE A: Físicas individuales y paredes */
        for (int i = 0; i < conf.n_particulas; i++) {
            actualizar_fisica_basica(&p[i], conf);
            resolver_colision_paredes(&p[i], conf);
        }

        /* BUCLE B: Colisiones entre partículas */
        for (int i = 0; i < conf.n_particulas; i++) {
            for (int j = i + 1; j < conf.n_particulas; j++) {
                resolver_colision_particulas(&p[i], &p[j]);
            }
        }

        /* BUCLE C: Extracción de datos y Energía */
        double ec = 0, ep = 0;
        for (int i = 0; i < conf.n_particulas; i++) {
            ec += 0.5 * p[i].m * (p[i].vx * p[i].vx + p[i].vy * p[i].vy);
            ep += p[i].m * conf.g * (conf.alto - p[i].y);

            fprintf(f_tray, "%d,%d,%.2f,%.2f,%.2f,%.2f,%.2f\n", t, i, p[i].x, p[i].y, p[i].vx,
                    p[i].vy, p[i].r);
        }
        fprintf(f_ener, "%d,%.2f,%.2f,%.2f\n", t, ec, ep, ec + ep);
    }
    /* ========================================== */

    /* 6. Benchmark */
    double total_t = (double)(clock() - inicio) / CLOCKS_PER_SEC;
    FILE *f_bench = fopen("benchmark.csv", "a");
    fprintf(f_bench, "%d,%d,%.4f\n", conf.n_particulas, conf.ticks, total_t);

    fclose(f_tray);
    fclose(f_ener);
    fclose(f_bench);
    printf("\nSimulacion completada en %.4fs.\n", total_t);

    return 0;
}