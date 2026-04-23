/* =============================================================
 * SIMULADOR DE N PARTÍCULAS EN 2D — SW-303 Lenguaje C
 * Archivo  : fisicas.c
 * Propósito: Implementación de física, movimiento y colisiones
 * Estándar : C99 / C11
 * Compilar : gcc -Wall -Wextra -std=c99 -O2 -lm
 * ============================================================= */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "particula.h"

/* ---------------------------------------------------------------
 * Constantes de inicialización
 * --------------------------------------------------------------- */
#define MAX_INTENTOS  2000    /* Intentos máximos para evitar solapamiento inicial */
#define RADIO_MIN      4.0    /* Radio mínimo de partícula (metros)                */
#define RADIO_MAX     12.0    /* Radio máximo de partícula (metros)                */
#define MASA_MIN       0.5    /* Masa mínima (kg)                                  */
#define MASA_MAX       3.0    /* Masa máxima (kg)                                  */
#define VEL_MAX      100.0    /* Velocidad inicial máxima en cada eje (m/s)        */

/* ---------------------------------------------------------------
 * aleatorio_doble()
 * Genera un número real en [min, max] usando rand().
 * --------------------------------------------------------------- */
static double aleatorio_doble(double minimo, double maximo) {
    return minimo + ((double)rand() / (double)RAND_MAX) * (maximo - minimo);
}

/* ---------------------------------------------------------------
 * valor_para_particula()
 * Devuelve el valor asignado a la partícula i según el modo y
 * alcance del ParametroInteractivo. Centraliza la lógica de
 * MANUAL vs ALEATORIO y GLOBAL vs POR_PARTICULA.
 *
 *   MANUAL + GLOBAL        → valor_global
 *   MANUAL + POR_PARTICULA → valores[i]  (debe estar asignado)
 *   ALEATORIO + GLOBAL     → valor_global (ya sorteado al configurar)
 *   ALEATORIO + POR_PARTICULA → nuevo rand() en [min, max]
 * --------------------------------------------------------------- */
static double valor_para_particula(const ParametroInteractivo* pp, int i) {
    if (pp->alcance == ALCANCE_GLOBAL) {
        return pp->valor_global;
    }
    /* POR_PARTICULA */
    if (pp->modo == MODO_MANUAL) {
        return pp->valores[i];
    }
    /* ALEATORIO por partícula: sorteo independiente por cada una */
    return aleatorio_doble(pp->min, pp->max);
}

/* ---------------------------------------------------------------
 * inicializar_particulas()
 * Asigna masa, radio, posición y velocidad a cada partícula usando
 * la configuración interactiva declarada en ConfigSimulacion.
 * La posición siempre se genera aleatoria dentro del rectángulo,
 * evitando solapamientos hasta MAX_INTENTOS reintentos.
 * --------------------------------------------------------------- */
void inicializar_particulas(Particula* p, int n, ConfigSimulacion* c) {
    int i, j;

    for (i = 0; i < n; i++) {
        /* Masa y radio respetando la configuración interactiva */
        p[i].masa  = valor_para_particula(&c->masa,  i);
        p[i].radio = valor_para_particula(&c->radio, i);

        /* Protección: evitar masa/radio no positivos (romperían la física) */
        if (p[i].masa  <= 0.0) p[i].masa  = MASA_MIN;
        if (p[i].radio <= 0.0) p[i].radio = RADIO_MIN;

        /* Intentar encontrar posición sin solapamiento */
        int intentos = 0;
        int valido   = 0;

        while (!valido && intentos < MAX_INTENTOS) {
            /* Posición aleatoria con margen igual al radio desde los bordes */
            p[i].x = aleatorio_doble(p[i].radio, c->ancho - p[i].radio);
            p[i].y = aleatorio_doble(p[i].radio, c->alto  - p[i].radio);

            /* Verificar que no se superponga con partículas ya colocadas */
            valido = 1;
            for (j = 0; j < i && valido; j++) {
                double dx   = p[i].x - p[j].x;
                double dy   = p[i].y - p[j].y;
                double dist = sqrt(dx * dx + dy * dy);
                /* Se exige 1 metro de separación extra como margen de seguridad */
                if (dist < p[i].radio + p[j].radio + 1.0) {
                    valido = 0;
                }
            }
            intentos++;
        }

        if (!valido) {
            fprintf(stderr,
                    "Advertencia: particula %d colocada con posible solapamiento "
                    "tras %d intentos.\n", i, MAX_INTENTOS);
        }

        /* Velocidad inicial.
         * Interpretación: el ParametroInteractivo 'velocidad' da el MÓDULO
         * (o la componente) en [min, max]; se distribuye con signo aleatorio
         * en cada eje para que aún se observe dispersión direccional.
         * Si el modo es MANUAL+GLOBAL con un único valor, ambos ejes reciben
         * ±valor con signo sorteado (50/50). */
        double vmod_x = valor_para_particula(&c->velocidad, i);
        double vmod_y = valor_para_particula(&c->velocidad, i);
        int signo_x = (rand() & 1) ? 1 : -1;
        int signo_y = (rand() & 1) ? 1 : -1;
        p[i].vx = signo_x * vmod_x;
        p[i].vy = signo_y * vmod_y;
    }
}

/* ---------------------------------------------------------------
 * aplicar_gravedad()
 * Aplica aceleración gravitacional constante hacia abajo (eje Y+).
 *
 * Ecuación de Euler para velocidad:
 *     vy(t + dt) = vy(t) + g * dt
 *
 * La componente horizontal no es afectada por la gravedad.
 * --------------------------------------------------------------- */
void aplicar_gravedad(Particula* p, int n, ConfigSimulacion* c) {
    int i;
    for (i = 0; i < n; i++) {
        p[i].vy += c->gravedad * c->dt;
    }
}

/* ---------------------------------------------------------------
 * aplicar_friccion()
 * Modela la resistencia viscosa del medio (aire/fluido).
 * La fuerza de arrastre es proporcional a la velocidad: F = -μ·v
 *
 * Ecuaciones discretizadas:
 *     vx(t + dt) = vx(t) * (1 - μ * dt)
 *     vy(t + dt) = vy(t) * (1 - μ * dt)
 *
 * Precaución: si μ·dt >= 1, el factor sería negativo (inversión de
 * velocidad) — se limita a cero para evitar comportamiento no físico.
 * --------------------------------------------------------------- */
void aplicar_friccion(Particula* p, int n, ConfigSimulacion* c) {
    double factor = 1.0 - c->friccion * c->dt;
    if (factor < 0.0) {
        factor = 0.0;  /* Limitar: la fricción no puede invertir la velocidad */
        fprintf(stderr, "Advertencia: friccion*dt >= 1. Reducir dt o friccion.\n");
    }

    int i;
    for (i = 0; i < n; i++) {
        p[i].vx *= factor;
        p[i].vy *= factor;
    }
}

/* ---------------------------------------------------------------
 * mover_particulas()
 * Integración de Euler hacia adelante (Forward Euler Method).
 * Actualiza la posición usando la velocidad del tick actual.
 *
 * Ecuaciones:
 *     x(t + dt) = x(t) + vx(t) * dt
 *     y(t + dt) = y(t) + vy(t) * dt
 *
 * Error de truncamiento: O(dt) por paso → O(dt²) acumulado.
 * Para mayor precisión se usaría Verlet o Runge-Kutta 4.
 * --------------------------------------------------------------- */
void mover_particulas(Particula* p, int n, double dt) {
    int i;
    for (i = 0; i < n; i++) {
        p[i].x += p[i].vx * dt;
        p[i].y += p[i].vy * dt;
    }
}

/* ---------------------------------------------------------------
 * colision_paredes()
 * Detecta cuando una partícula sale del espacio delimitado y
 * aplica reflexión con el coeficiente de restitución configurado.
 *
 * Reflexión (ejemplo pared derecha):
 *     Si x + radio > ancho:
 *         vx' = -|vx| * e    (siempre apunta hacia adentro)
 *         x   = ancho - radio (reposicionar dentro del límite)
 *
 * El uso de fabs() garantiza que vx siempre apunte hacia el interior,
 * previniendo que partículas "escapen" si ya penetraron la pared.
 * --------------------------------------------------------------- */
void colision_paredes(Particula* p, int n, ConfigSimulacion* c) {
    double e = c->coef_restitucion;
    int i;

    for (i = 0; i < n; i++) {
        /* Pared izquierda: x = 0 */
        if (p[i].x - p[i].radio < 0.0) {
            p[i].vx = fabs(p[i].vx) * e;      /* reflejar hacia la derecha */
            p[i].x  = p[i].radio;              /* reposicionar              */
        }
        /* Pared derecha: x = ancho */
        if (p[i].x + p[i].radio > c->ancho) {
            p[i].vx = -fabs(p[i].vx) * e;     /* reflejar hacia la izquierda */
            p[i].x  = c->ancho - p[i].radio;
        }
        /* Pared superior: y = 0 */
        if (p[i].y - p[i].radio < 0.0) {
            p[i].vy = fabs(p[i].vy) * e;      /* reflejar hacia abajo */
            p[i].y  = p[i].radio;
        }
        /* Pared inferior: y = alto */
        if (p[i].y + p[i].radio > c->alto) {
            p[i].vy = -fabs(p[i].vy) * e;     /* reflejar hacia arriba */
            p[i].y  = c->alto - p[i].radio;
        }
    }
}

/* ---------------------------------------------------------------
 * resolver_colision_elastica()   [función estática — uso interno]
 * Aplica la respuesta de colisión elástica 2D entre dos partículas.
 *
 * Algoritmo (método del impulso):
 *
 *  1. Vector normal unitario de p1 hacia p2:
 *         n = (p2.pos - p1.pos) / |p2.pos - p1.pos|
 *
 *  2. Velocidad relativa proyectada sobre la normal:
 *         vrel_n = (v1 - v2) · n
 *         Si vrel_n ≤ 0: las partículas se alejan → no resolver
 *
 *  3. Impulso de colisión elástica (conserva momento y Ek):
 *         J = 2 * m1 * m2 * vrel_n / (m1 + m2)
 *
 *  4. Aplicar impulso (igual y opuesto: conserva momento total):
 *         v1' = v1 - (J / m1) * n
 *         v2' = v2 + (J / m2) * n
 *
 *  5. Corrección de solapamiento (overlap correction):
 *         Separar centros para que dist = r1 + r2
 * --------------------------------------------------------------- */
static void resolver_colision_elastica(Particula* p1, Particula* p2) {
    /* 1. Vector de separación entre centros */
    double dx   = p2->x - p1->x;
    double dy   = p2->y - p1->y;
    double dist = sqrt(dx * dx + dy * dy);

    /* Evitar división por cero si los centros coinciden exactamente */
    if (dist < 1e-10) {
        p1->x -= 0.5;
        p2->x += 0.5;
        return;
    }

    /* 2. Vector normal unitario (de p1 → p2) */
    double nx = dx / dist;
    double ny = dy / dist;

    /* 3. Velocidad relativa proyectada sobre la dirección normal */
    double dvx    = p1->vx - p2->vx;
    double dvy    = p1->vy - p2->vy;
    double vrel_n = dvx * nx + dvy * ny;

    /* Si las partículas ya se alejan, no hay colisión a resolver */
    if (vrel_n <= 0.0) {
        return;
    }

    /* 4. Impulso escalar J para colisión elástica */
    double masa_total = p1->masa + p2->masa;
    double J          = (2.0 * p1->masa * p2->masa * vrel_n) / masa_total;

    /* 5. Aplicar impulso con signo opuesto → conserva momento lineal */
    p1->vx -= (J / p1->masa) * nx;
    p1->vy -= (J / p1->masa) * ny;
    p2->vx += (J / p2->masa) * nx;
    p2->vy += (J / p2->masa) * ny;

    /* 6. Corrección de solapamiento: alejar partículas hasta que no se toquen */
    double solapamiento = (p1->radio + p2->radio) - dist;
    if (solapamiento > 0.0) {
        double correc_x = (solapamiento * 0.5) * nx;
        double correc_y = (solapamiento * 0.5) * ny;
        p1->x -= correc_x;
        p1->y -= correc_y;
        p2->x += correc_x;
        p2->y += correc_y;
    }
}

/* ---------------------------------------------------------------
 * detectar_y_resolver_colisiones()
 * Algoritmo O(N²): evalúa todos los pares únicos de partículas.
 *
 * Número de comparaciones por tick: N*(N-1)/2
 *   N=50:    1,225 comparaciones
 *   N=100:   4,950 comparaciones
 *   N=500: 124,750 comparaciones
 *
 * Optimización incluida: compara dist² vs (r1+r2)² para evitar
 * llamadas costosas a sqrt() cuando no hay colisión (caso mayoritario).
 * El sqrt solo se ejecuta si hay colisión efectiva.
 * --------------------------------------------------------------- */
void detectar_y_resolver_colisiones(Particula* p, int n) {
    int i, j;
    for (i = 0; i < n - 1; i++) {
        for (j = i + 1; j < n; j++) {
            double dx          = p[j].x - p[i].x;
            double dy          = p[j].y - p[i].y;
            double dist2       = dx * dx + dy * dy;           /* sin sqrt */
            double suma_radios = p[i].radio + p[j].radio;

            /* Condición: dist < r1+r2 ↔ dist² < (r1+r2)² */
            if (dist2 < suma_radios * suma_radios) {
                /* Colisión confirmada → resolver como elástica 2D */
                resolver_colision_elastica(&p[i], &p[j]);
            }
        }
    }
}

/* ---------------------------------------------------------------
 * calcular_energia_cinetica()
 * Suma la energía cinética de todas las partículas.
 *
 * Fórmula: Ek = Σ (½ * mᵢ * (vxᵢ² + vyᵢ²))
 * Unidades: Joules (J = kg·m²/s²)
 * --------------------------------------------------------------- */
double calcular_energia_cinetica(Particula* p, int n) {
    double ek = 0.0;
    int i;
    for (i = 0; i < n; i++) {
        ek += 0.5 * p[i].masa * (p[i].vx * p[i].vx + p[i].vy * p[i].vy);
    }
    return ek;
}

/* ---------------------------------------------------------------
 * calcular_energia_potencial()
 * Suma la energía potencial gravitacional de todas las partículas.
 *
 * Fórmula: Ep = Σ (mᵢ * g * hᵢ)
 *   donde hᵢ = alto - yᵢ  (altura real: Y=0 está arriba)
 *
 * Con (alto - y): cuando y aumenta (cae), la altura decrece y
 * Ep decrece → Ek aumenta → conservación de Et en sistema ideal.
 * --------------------------------------------------------------- */
double calcular_energia_potencial(Particula* p, int n, double g, double alto) {
    double ep = 0.0;
    int i;
    for (i = 0; i < n; i++) {
        double altura = alto - p[i].y;        /* altura real sobre el suelo */
        ep += p[i].masa * g * altura;
    }
    return ep;
}
