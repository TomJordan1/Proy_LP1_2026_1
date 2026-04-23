/* =============================================================
 * SIMULADOR DE N PARTÍCULAS EN 2D — SW-303 Lenguaje C
 * Archivo  : simulador.c
 * Propósito: Programa principal — loop de simulación y orquestación
 * Estándar : C99 / C11
 *
 * Compilar : gcc -Wall -Wextra -std=c99 -O2 src/simulador.c src/fisicas.c src/csv_export.c -o simulador -lm
 * Uso      : ./simulador [n_particulas] [ticks_totales]
 * Ejemplo  : ./simulador 50 500
 * ============================================================= */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "particula.h"

/* ---------------------------------------------------------------
 * PARÁMETROS POR DEFECTO DEL SISTEMA
 * Todos son modificables en tiempo de compilación o por argumentos.
 * --------------------------------------------------------------- */
#define N_PARTICULAS_DEFAULT     50       /* Número de partículas                */
#define TICKS_DEFAULT           500       /* Iteraciones totales de simulación   */
#define GRAVEDAD_DEFAULT       9.81       /* Aceleración gravitacional (m/s²)    */
#define FRICCION_DEFAULT       0.01       /* Coeficiente de fricción viscosa     */
#define DT_DEFAULT             0.016      /* Paso de tiempo ~60 fps (s)          */
#define ANCHO_DEFAULT        800.0        /* Ancho del espacio (m)               */
#define ALTO_DEFAULT         600.0        /* Alto del espacio (m)                */
#define RESTITUCION_DEFAULT    0.85       /* Coeficiente de rebote en paredes    */
#define PROGRESO_INTERVALO    100         /* Imprimir progreso cada N ticks      */

/* --- Nombres de archivos de salida --- */
#define ARCHIVO_TRAY    "trayectorias.csv"
#define ARCHIVO_ENER    "energia.csv"
#define ARCHIVO_BENCH   "benchmark.csv"

/* ---------------------------------------------------------------
 * imprimir_configuracion()
 * Muestra en consola los parámetros activos antes de simular.
 * --------------------------------------------------------------- */
/* Texto corto para describir un ParametroInteractivo */
static const char* modo_str(ModoEntrada m) {
    return (m == MODO_MANUAL) ? "MANUAL" : "ALEATORIO";
}
static const char* alcance_str(AlcanceValor a) {
    return (a == ALCANCE_GLOBAL) ? "GLOBAL" : "POR_PART";
}

static void imprimir_configuracion(const ConfigSimulacion* c) {
    printf("====================================================\n");
    printf("   Simulador de N Particulas en 2D -- SW-303\n");
    printf("====================================================\n");
    printf("  Particulas     : %d  (%s)\n", c->n_particulas, modo_str(c->modo_n));
    printf("  Ticks totales  : %d\n",        c->ticks_totales);
    printf("  Tiempo total   : %.3f s\n",    c->ticks_totales * c->dt);
    printf("  Gravedad       : %.2f m/s2  (%s)\n",
                                          c->gravedad, modo_str(c->modo_gravedad));
    printf("  Friccion       : %.4f s-1\n",  c->friccion);
    printf("  dt             : %.4f s\n",    c->dt);
    printf("  Espacio        : %.0f x %.0f m\n", c->ancho, c->alto);
    printf("  Restitucion    : %.2f\n",      c->coef_restitucion);
    printf("  Velocidad      : %s/%s  rango=[%.2f,%.2f]\n",
                   modo_str(c->velocidad.modo), alcance_str(c->velocidad.alcance),
                   c->velocidad.min, c->velocidad.max);
    printf("  Masa           : %s/%s  rango=[%.2f,%.2f]\n",
                   modo_str(c->masa.modo), alcance_str(c->masa.alcance),
                   c->masa.min, c->masa.max);
    printf("  Radio          : %s/%s  rango=[%.2f,%.2f]\n",
                   modo_str(c->radio.modo), alcance_str(c->radio.alcance),
                   c->radio.min, c->radio.max);
    printf("====================================================\n\n");
}

/* ---------------------------------------------------------------
 * main()
 * Punto de entrada del programa.
 *
 * Flujo de ejecución:
 *   1. Parsear argumentos opcionales de línea de comandos
 *   2. Configurar parámetros de simulación
 *   3. Reservar memoria dinámica para partículas
 *   4. Inicializar partículas (posición y velocidad aleatoria)
 *   5. Abrir archivos CSV de salida
 *   6. Ejecutar loop principal (tick a tick)
 *   7. Cerrar archivos, medir tiempo, exportar benchmark
 *   8. Liberar memoria y terminar
 * --------------------------------------------------------------- */
int main(int argc, char* argv[]) {
    /* === 1. Semilla aleatoria === */
    srand((unsigned int)time(NULL));

    /* === 2. Iniciar temporizador de rendimiento === */
    clock_t inicio = clock();

    /* === 3. Configuración con valores por defecto === */
    ConfigSimulacion config;
    config.n_particulas     = N_PARTICULAS_DEFAULT;
    config.gravedad         = GRAVEDAD_DEFAULT;
    config.friccion         = FRICCION_DEFAULT;
    config.dt               = DT_DEFAULT;
    config.ticks_totales    = TICKS_DEFAULT;
    config.ancho            = ANCHO_DEFAULT;
    config.alto             = ALTO_DEFAULT;
    config.coef_restitucion = RESTITUCION_DEFAULT;

    /* Inicializar punteros dinámicos a NULL para que liberar_parametros()
     * sea siempre seguro incluso si el usuario salta la configuración. */
    config.velocidad.valores = NULL;
    config.masa.valores      = NULL;
    config.radio.valores     = NULL;

    /* === 4. Leer argumentos opcionales: ./simulador [N] [ticks] === */
    if (argc >= 2) {
        int n_arg = atoi(argv[1]);
        if (n_arg < 2) {
            fprintf(stderr, "Error: se requieren al menos 2 particulas.\n");
            fprintf(stderr, "Uso: %s [n_particulas] [ticks]\n", argv[0]);
            return EXIT_FAILURE;
        }
        config.n_particulas = n_arg;
    }

    if (argc >= 3) {
        int t_arg = atoi(argv[2]);
        if (t_arg < 1) {
            fprintf(stderr, "Error: se requiere al menos 1 tick.\n");
            return EXIT_FAILURE;
        }
        config.ticks_totales = t_arg;
    }

    /* === 4.b Configuración interactiva de parámetros ===
     * El usuario elige MANUAL/ALEATORIO para N, gravedad, velocidad, masa,
     * radio; y GLOBAL/POR_PARTICULA cuando aplica. Puede saltar el menú
     * con 'N' para mantener el comportamiento histórico del simulador. */
    configurar_interactivamente(&config);

    /* Mostrar configuración activa */
    imprimir_configuracion(&config);

    /* === 5. Asignación dinámica de memoria para partículas === */
    Particula* particulas = (Particula*)calloc(
        (size_t)config.n_particulas, sizeof(Particula));

    if (!particulas) {
        fprintf(stderr,
                "Error: no se pudo reservar memoria para %d particulas.\n"
                "Intente reducir n_particulas.\n",
                config.n_particulas);
        return EXIT_FAILURE;
    }

    /* === 6. Inicializar partículas === */
    inicializar_particulas(particulas, config.n_particulas, &config);
    printf("OK Particulas inicializadas (%d)\n", config.n_particulas);

    /* === 7. Abrir archivos CSV === */
    FILE* f_tray = abrir_csv_trayectorias(ARCHIVO_TRAY);
    FILE* f_ener = abrir_csv_energia(ARCHIVO_ENER);
    printf("OK Archivos CSV creados: %s, %s\n\n", ARCHIVO_TRAY, ARCHIVO_ENER);

    /* =========================================================
     * LOOP PRINCIPAL DE SIMULACIÓN
     *
     * Orden de operaciones por tick (justificado en entregable 1):
     *   1. Gravedad   → actualiza vy antes de mover
     *   2. Fricción   → atenúa velocidad post-fuerzas externas
     *   3. Movimiento → integración de Euler con velocidad actualizada
     *   4. Paredes    → reflejar y reposicionar antes de inter-colisiones
     *   5. Colisiones → resolver pares (O(N²))
     *   6. Energía    → medir estado final del tick
     *   7. CSV        → persistir estado consistente
     * ========================================================= */
    printf("Simulando %d ticks...\n", config.ticks_totales);

    int tick;
    for (tick = 0; tick < config.ticks_totales; tick++) {

        /* PASO 1: Aplicar gravedad — vy += g * dt */
        aplicar_gravedad(particulas, config.n_particulas, &config);

        /* PASO 2: Aplicar fricción viscosa — v *= (1 - mu*dt) */
        aplicar_friccion(particulas, config.n_particulas, &config);

        /* PASO 3: Integración de Euler — x += vx*dt, y += vy*dt */
        mover_particulas(particulas, config.n_particulas, config.dt);

        /* PASO 4: Colisiones con las cuatro paredes (reflexión) */
        colision_paredes(particulas, config.n_particulas, &config);

        /* PASO 5: Detectar y resolver colisiones entre pares O(N²) */
        detectar_y_resolver_colisiones(particulas, config.n_particulas);

        /* PASO 6: Calcular energías del sistema en este estado */
        double ek = calcular_energia_cinetica(particulas, config.n_particulas);
        double ep = calcular_energia_potencial(particulas, config.n_particulas,
                                               config.gravedad, config.alto);
        double et = ek + ep;

        /* PASO 7: Exportar estado a CSV */
        exportar_tick_trayectorias(f_tray, tick, particulas, config.n_particulas);
        exportar_tick_energia(f_ener, tick, ek, ep, et);

        /* Progreso en consola cada PROGRESO_INTERVALO ticks */
        if ((tick + 1) % PROGRESO_INTERVALO == 0
            || tick == config.ticks_totales - 1) {
            printf("  Tick %4d/%d | Ek=%10.2f J | Ep=%10.2f J | Et=%10.2f J\n",
                   tick + 1, config.ticks_totales, ek, ep, et);
        }
    }

    /* === 8. Cerrar archivos CSV === */
    fclose(f_tray);
    fclose(f_ener);

    /* === 9. Calcular y reportar rendimiento === */
    double tiempo_total = (double)(clock() - inicio) / (double)CLOCKS_PER_SEC;
    double tasa         = (tiempo_total > 1e-9)
                          ? ((double)config.n_particulas * config.ticks_totales)
                            / tiempo_total
                          : 0.0;

    /* === 10. Exportar benchmark (modo append para acumular corridas) === */
    exportar_benchmark(ARCHIVO_BENCH,
                       config.n_particulas,
                       config.ticks_totales,
                       tiempo_total);

    /* === 11. Resumen final en consola === */
    printf("\n====================================================\n");
    printf("  Simulacion completada exitosamente.\n");
    printf("  Tiempo de ejecucion : %.4f segundos\n", tiempo_total);
    printf("  Tasa de proceso     : %.0f part*ticks/s\n", tasa);
    printf("  Colisiones/tick     : %d pares evaluados\n",
           config.n_particulas * (config.n_particulas - 1) / 2);
    printf("====================================================\n");
    printf("Archivo CSV generado: %s\n", ARCHIVO_TRAY);
    printf("Archivo CSV generado: %s\n", ARCHIVO_ENER);

    /* === 12. Liberar toda la memoria dinámica === */
    free(particulas);
    particulas = NULL;   /* buena práctica: evitar puntero colgante */

    /* Liberar arrays .valores de ConfigSimulacion (si los hubo) */
    liberar_parametros(&config);

    return EXIT_SUCCESS;
}
