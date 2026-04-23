/* =============================================================
 * SIMULADOR DE N PARTÍCULAS EN 2D — SW-303 Lenguaje C
 * Archivo  : entrada.c
 * Propósito: Menú interactivo para configurar parámetros del sistema
 *            antes de la simulación. Cada parámetro puede ser
 *            MANUAL (escrito por el usuario) o ALEATORIO (generado
 *            por rand() en un rango), y los parámetros por partícula
 *            pueden ser GLOBAL (mismo valor para todas) o
 *            POR_PARTICULA (un valor distinto por cada una).
 * Estándar : C99 / C11
 * Compilar : se construye junto al resto del proyecto con el Makefile
 *            (usa wildcard sobre archivos .c en src/), sin pasos adicionales.
 * ============================================================= */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "particula.h"

/* ---------------------------------------------------------------
 * Rangos por defecto para los modos ALEATORIO (valores del dominio
 * físico original del simulador — mantienen comportamiento histórico).
 * --------------------------------------------------------------- */
#define N_MIN_DEFAULT           2       /* mínimo N razonable para ver colisiones */
#define N_MAX_DEFAULT         200       /* máximo N razonable al pedir aleatorio   */
#define VEL_MIN_DEFAULT      -100.0     /* m/s                                     */
#define VEL_MAX_DEFAULT       100.0
#define MASA_MIN_DEFAULT        0.5     /* kg                                      */
#define MASA_MAX_DEFAULT        3.0
#define RADIO_MIN_DEFAULT       4.0     /* metros                                  */
#define RADIO_MAX_DEFAULT      12.0
#define GRAV_MIN_DEFAULT        0.0     /* m/s²                                    */
#define GRAV_MAX_DEFAULT       20.0
#define GRAV_DEFAULT            9.81    /* m/s² — gravedad terrestre               */

/* ---------------------------------------------------------------
 * leer_linea()
 * Lee una línea completa de stdin en buffer (tam bytes).
 * Devuelve 1 si tuvo éxito, 0 en EOF.
 * Elimina el salto de línea final si existe.
 * --------------------------------------------------------------- */
static int leer_linea(char* buffer, int tam) {
    if (fgets(buffer, tam, stdin) == NULL) {
        return 0;
    }
    size_t len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\n') {
        buffer[len - 1] = '\0';
    }
    return 1;
}

/* ---------------------------------------------------------------
 * preguntar_modo()
 * Pregunta al usuario si el valor del parámetro será manual o
 * aleatorio. Vuelve a preguntar ante entrada inválida.
 * --------------------------------------------------------------- */
static ModoEntrada preguntar_modo(const char* etiqueta) {
    char buffer[64];
    for (;;) {
        printf("  >> %s: [M]anual o [A]leatorio? ", etiqueta);
        fflush(stdout);
        if (!leer_linea(buffer, sizeof buffer)) {
            /* EOF: asumimos aleatorio por defecto para no colgar */
            printf("(EOF detectado, usando ALEATORIO)\n");
            return MODO_ALEATORIO;
        }
        if (buffer[0] == 'M' || buffer[0] == 'm') return MODO_MANUAL;
        if (buffer[0] == 'A' || buffer[0] == 'a') return MODO_ALEATORIO;
        printf("     Entrada invalida. Escriba 'M' o 'A'.\n");
    }
}

/* ---------------------------------------------------------------
 * preguntar_alcance()
 * Pregunta si el valor del parámetro será global (mismo para todas
 * las partículas) o por partícula (uno distinto por cada una).
 * --------------------------------------------------------------- */
static AlcanceValor preguntar_alcance(const char* etiqueta) {
    char buffer[64];
    for (;;) {
        printf("  >> %s: [G]lobal (igual para todas) o [P]or particula? ", etiqueta);
        fflush(stdout);
        if (!leer_linea(buffer, sizeof buffer)) {
            printf("(EOF detectado, usando GLOBAL)\n");
            return ALCANCE_GLOBAL;
        }
        if (buffer[0] == 'G' || buffer[0] == 'g') return ALCANCE_GLOBAL;
        if (buffer[0] == 'P' || buffer[0] == 'p') return ALCANCE_POR_PARTICULA;
        printf("     Entrada invalida. Escriba 'G' o 'P'.\n");
    }
}

/* ---------------------------------------------------------------
 * leer_double()
 * Lee un double de stdin con mensaje y validación de rango mínimo.
 * Repite hasta obtener un valor válido.
 * --------------------------------------------------------------- */
static double leer_double(const char* mensaje, double minimo, double maximo) {
    char buffer[128];
    double v;
    for (;;) {
        printf("     %s [%.3f, %.3f]: ", mensaje, minimo, maximo);
        fflush(stdout);
        if (!leer_linea(buffer, sizeof buffer)) {
            printf("(EOF: usando %.3f)\n", minimo);
            return minimo;
        }
        if (sscanf(buffer, "%lf", &v) == 1 && v >= minimo && v <= maximo) {
            return v;
        }
        printf("     Valor invalido. Debe estar entre %.3f y %.3f.\n", minimo, maximo);
    }
}

/* ---------------------------------------------------------------
 * leer_int()
 * Lee un entero de stdin con validación de rango.
 * --------------------------------------------------------------- */
static int leer_int(const char* mensaje, int minimo, int maximo) {
    char buffer[128];
    int v;
    for (;;) {
        printf("     %s [%d, %d]: ", mensaje, minimo, maximo);
        fflush(stdout);
        if (!leer_linea(buffer, sizeof buffer)) {
            printf("(EOF: usando %d)\n", minimo);
            return minimo;
        }
        if (sscanf(buffer, "%d", &v) == 1 && v >= minimo && v <= maximo) {
            return v;
        }
        printf("     Valor invalido. Debe ser entero entre %d y %d.\n", minimo, maximo);
    }
}

/* ---------------------------------------------------------------
 * aleatorio_uniforme()
 * Genera un double uniforme en [min, max].
 * --------------------------------------------------------------- */
static double aleatorio_uniforme(double minimo, double maximo) {
    return minimo + ((double)rand() / (double)RAND_MAX) * (maximo - minimo);
}

/* ---------------------------------------------------------------
 * configurar_parametro()
 * Función genérica que configura un ParametroInteractivo preguntando
 * por modo y alcance. Para alcance POR_PARTICULA, asigna el array
 * dinámico y llena los valores (manual o aleatorio según el modo).
 *
 * Parámetros:
 *   p         : destino de la configuración
 *   etiqueta  : nombre del atributo para los prompts (ej. "Masa")
 *   n         : número de partículas (necesario si POR_PARTICULA)
 *   mn, mx    : rango por defecto ofrecido al usuario
 *   permitir_negativo : true para velocidades, false para masa/radio
 * --------------------------------------------------------------- */
static void configurar_parametro(ParametroInteractivo* p,
                                 const char* etiqueta,
                                 int n,
                                 double mn, double mx,
                                 int permitir_negativo)
{
    printf("\n-- Configurar %s --\n", etiqueta);
    p->modo    = preguntar_modo(etiqueta);
    p->alcance = preguntar_alcance(etiqueta);
    p->valores = NULL;

    /* Rango físico razonable (se puede ajustar en modo aleatorio) */
    double rango_min = permitir_negativo ? mn : (mn < 0.0 ? 0.0 : mn);
    double rango_max = mx;

    if (p->modo == MODO_MANUAL && p->alcance == ALCANCE_GLOBAL) {
        /* Un único valor tipeado, aplicado a todas las partículas */
        p->valor_global = leer_double("Valor unico", rango_min, rango_max);
        p->min = p->max = p->valor_global;

    } else if (p->modo == MODO_MANUAL && p->alcance == ALCANCE_POR_PARTICULA) {
        /* Pedir N valores al usuario, uno por partícula */
        p->valores = (double*)malloc((size_t)n * sizeof(double));
        if (!p->valores) {
            fprintf(stderr, "Error: sin memoria para %s por particula.\n", etiqueta);
            exit(EXIT_FAILURE);
        }
        printf("     Ingrese %d valores (uno por particula).\n", n);
        for (int i = 0; i < n; i++) {
            char msg[64];
            snprintf(msg, sizeof msg, "  [particula %d/%d] %s", i + 1, n, etiqueta);
            p->valores[i] = leer_double(msg, rango_min, rango_max);
        }
        p->min = rango_min;
        p->max = rango_max;

    } else if (p->modo == MODO_ALEATORIO && p->alcance == ALCANCE_GLOBAL) {
        /* Un único aleatorio, mismo para todas. Permitir ajustar rango. */
        printf("     (rango aleatorio; Enter para usar defaults: [%.3f, %.3f])\n",
               rango_min, rango_max);
        p->min = leer_double("Minimo del rango", rango_min, rango_max);
        p->max = leer_double("Maximo del rango", p->min,    rango_max);
        p->valor_global = aleatorio_uniforme(p->min, p->max);
        printf("     ==> valor global generado: %.4f\n", p->valor_global);

    } else { /* ALEATORIO + POR_PARTICULA */
        /* Cada partícula recibirá su valor en inicializar_particulas().
         * Aquí solo fijamos el rango; el array .valores se mantiene NULL. */
        printf("     (rango aleatorio por particula; Enter para usar defaults)\n");
        p->min = leer_double("Minimo del rango", rango_min, rango_max);
        p->max = leer_double("Maximo del rango", p->min,    rango_max);
    }
}

/* ---------------------------------------------------------------
 * configurar_interactivamente()
 * Función pública: orquesta el menú completo.
 * --------------------------------------------------------------- */
void configurar_interactivamente(ConfigSimulacion* c) {
    char buffer[64];

    printf("\n====================================================\n");
    printf("   CONFIGURACION INTERACTIVA DE PARAMETROS\n");
    printf("   (Presione 's' para saltar y usar valores por defecto)\n");
    printf("====================================================\n");
    printf("Usar configuracion interactiva? [S]i/[N]o: ");
    fflush(stdout);
    if (leer_linea(buffer, sizeof buffer)) {
        if (buffer[0] == 'N' || buffer[0] == 'n') {
            /* Saltar: dejar valores por defecto (modo aleatorio, rangos originales) */
            c->modo_n        = MODO_ALEATORIO;
            c->modo_gravedad = MODO_MANUAL;  /* gravedad queda con el default */
            c->velocidad.modo    = MODO_ALEATORIO;
            c->velocidad.alcance = ALCANCE_POR_PARTICULA;
            c->velocidad.min     = VEL_MIN_DEFAULT;
            c->velocidad.max     = VEL_MAX_DEFAULT;
            c->velocidad.valores = NULL;
            c->masa.modo    = MODO_ALEATORIO;
            c->masa.alcance = ALCANCE_POR_PARTICULA;
            c->masa.min     = MASA_MIN_DEFAULT;
            c->masa.max     = MASA_MAX_DEFAULT;
            c->masa.valores = NULL;
            c->radio.modo    = MODO_ALEATORIO;
            c->radio.alcance = ALCANCE_POR_PARTICULA;
            c->radio.min     = RADIO_MIN_DEFAULT;
            c->radio.max     = RADIO_MAX_DEFAULT;
            c->radio.valores = NULL;
            printf("OK Usando configuracion por defecto (comportamiento historico).\n");
            return;
        }
    }

    /* -------- 1) Numero de particulas -------- */
    printf("\n-- Configurar NUMERO DE PARTICULAS --\n");
    c->modo_n = preguntar_modo("Numero de particulas");
    if (c->modo_n == MODO_MANUAL) {
        c->n_particulas = leer_int("Cuantas particulas", N_MIN_DEFAULT, 5000);
    } else {
        int mn = leer_int("Minimo", N_MIN_DEFAULT, N_MAX_DEFAULT);
        int mx = leer_int("Maximo", mn, N_MAX_DEFAULT);
        c->n_particulas = mn + rand() % (mx - mn + 1);
        printf("     ==> N generado aleatoriamente: %d\n", c->n_particulas);
    }

    /* -------- 2) Gravedad (escalar del sistema) -------- */
    printf("\n-- Configurar GRAVEDAD --\n");
    c->modo_gravedad = preguntar_modo("Gravedad");
    if (c->modo_gravedad == MODO_MANUAL) {
        printf("     (valor por defecto fisico terrestre: %.2f m/s^2)\n", GRAV_DEFAULT);
        c->gravedad = leer_double("Gravedad m/s^2", GRAV_MIN_DEFAULT, GRAV_MAX_DEFAULT);
    } else {
        double mn = leer_double("Minimo g", GRAV_MIN_DEFAULT, GRAV_MAX_DEFAULT);
        double mx = leer_double("Maximo g", mn,               GRAV_MAX_DEFAULT);
        c->gravedad = aleatorio_uniforme(mn, mx);
        printf("     ==> Gravedad generada aleatoriamente: %.4f m/s^2\n", c->gravedad);
    }

    /* -------- 3) Velocidad (por partícula, con signo aleatorio) -------- */
    configurar_parametro(&c->velocidad, "Velocidad (m/s)", c->n_particulas,
                         VEL_MIN_DEFAULT, VEL_MAX_DEFAULT, 1 /* permitir negativo */);

    /* -------- 4) Masa -------- */
    configurar_parametro(&c->masa, "Masa (kg)", c->n_particulas,
                         MASA_MIN_DEFAULT, MASA_MAX_DEFAULT, 0);

    /* -------- 5) Radio -------- */
    configurar_parametro(&c->radio, "Radio (m)", c->n_particulas,
                         RADIO_MIN_DEFAULT, RADIO_MAX_DEFAULT, 0);

    printf("\nOK Configuracion interactiva completada.\n");
}

/* ---------------------------------------------------------------
 * liberar_parametros()
 * Libera los arrays .valores asignados dinámicamente si los hay.
 * --------------------------------------------------------------- */
void liberar_parametros(ConfigSimulacion* c) {
    if (c->velocidad.valores) { free(c->velocidad.valores); c->velocidad.valores = NULL; }
    if (c->masa.valores)      { free(c->masa.valores);      c->masa.valores      = NULL; }
    if (c->radio.valores)     { free(c->radio.valores);     c->radio.valores     = NULL; }
}
