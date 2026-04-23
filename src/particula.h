/* =============================================================
 * SIMULADOR DE N PARTÍCULAS EN 2D — SW-303 Lenguaje C
 * Archivo  : particula.h
 * Propósito: Definición de structs y prototipos de funciones
 * Estándar : C99 / C11
 * ============================================================= */
#ifndef PARTICULA_H
#define PARTICULA_H

#include <stdio.h>

/* ---------------------------------------------------------------
 * Struct Particula
 * Representa una partícula del sistema con su estado cinemático.
 * Todos los valores en unidades SI (metros, kg, m/s).
 * --------------------------------------------------------------- */
typedef struct {
    double x;      /* Posición horizontal del centro (m), 0=izquierda */
    double y;      /* Posición vertical del centro (m), 0=arriba       */
    double vx;     /* Velocidad en eje X (m/s), positivo = derecha     */
    double vy;     /* Velocidad en eje Y (m/s), positivo = abajo       */
    double masa;   /* Masa inercial (kg), usada en fórmula de impulso  */
    double radio;  /* Radio de la partícula para detección de colisión  */
} Particula;

/* ---------------------------------------------------------------
 * ENTRADA INTERACTIVA
 * Enumeraciones para controlar cómo el usuario especifica cada
 * parámetro físico antes de ejecutar la simulación.
 * --------------------------------------------------------------- */

/* Modo de entrada del valor: lo escribe el usuario o lo genera el RNG */
typedef enum {
    MODO_ALEATORIO = 0,   /* valor(es) generado(s) por rand() en un rango  */
    MODO_MANUAL    = 1    /* valor(es) ingresado(s) directamente por teclado */
} ModoEntrada;

/* Alcance: un valor único para todas las partículas, o uno distinto por cada una */
typedef enum {
    ALCANCE_GLOBAL        = 0,   /* mismo valor para las N partículas       */
    ALCANCE_POR_PARTICULA = 1    /* un valor distinto para cada partícula    */
} AlcanceValor;

/* ---------------------------------------------------------------
 * Struct ParametroInteractivo
 * Empaqueta la configuración de entrada para un atributo físico
 * (velocidad, masa o radio) de las partículas.
 *
 * Combinaciones válidas:
 *   MANUAL + GLOBAL        → valor_global aplicado a todas
 *   MANUAL + POR_PARTICULA → array 'valores' leído del teclado (N entradas)
 *   ALEATORIO + GLOBAL     → un rand() en [min,max], mismo para todas
 *   ALEATORIO + POR_PARTICULA → rand() independiente por partícula en [min,max]
 * --------------------------------------------------------------- */
typedef struct {
    ModoEntrada  modo;           /* MANUAL o ALEATORIO                              */
    AlcanceValor alcance;        /* GLOBAL o POR_PARTICULA                           */
    double       valor_global;   /* usado cuando alcance = GLOBAL                   */
    double       min;            /* límite inferior del rango aleatorio              */
    double       max;            /* límite superior del rango aleatorio              */
    double*      valores;        /* array[N] usado cuando alcance = POR_PARTICULA    */
} ParametroInteractivo;

/* ---------------------------------------------------------------
 * Struct ConfigSimulacion
 * Parámetros globales que controlan el comportamiento del sistema.
 *
 * Los atributos físicos "por partícula" (velocidad, masa, radio) se
 * configuran via structs ParametroInteractivo. La gravedad es un
 * escalar del sistema (no por partícula) pero admite modo manual o
 * aleatorio en un rango razonable (p.ej. 0–20 m/s²).
 * --------------------------------------------------------------- */
typedef struct {
    int    n_particulas;      /* Número de partículas simuladas (N ≥ 2)          */
    ModoEntrada modo_n;       /* MANUAL o ALEATORIO para N                        */

    double gravedad;          /* Aceleración gravitacional g (m/s²), default 9.81 */
    ModoEntrada modo_gravedad;/* MANUAL o ALEATORIO para g                        */

    double friccion;          /* Coeficiente de fricción viscosa μ (s⁻¹)          */
    double dt;                /* Paso de tiempo por tick (s), default 0.016       */
    int    ticks_totales;     /* Número total de iteraciones de la simulación      */
    double ancho;             /* Ancho del espacio de simulación (m)              */
    double alto;              /* Alto del espacio de simulación (m)               */
    double coef_restitucion;  /* Coeficiente de rebote en paredes [0.0, 1.0]      */

    /* Atributos físicos que pueden ser manual/aleatorio y global/por partícula */
    ParametroInteractivo velocidad;  /* módulo aplicado a vx y vy (con signo aleatorio) */
    ParametroInteractivo masa;       /* masa en kg                                       */
    ParametroInteractivo radio;      /* radio en metros                                  */
} ConfigSimulacion;

/* =============================================================
 * PROTOTIPOS — entrada.c  (menú interactivo de parámetros)
 * ============================================================= */

/**
 * Muestra menú interactivo por stdout y llena los campos de entrada
 * interactiva de ConfigSimulacion. Para cada parámetro (N, velocidad,
 * masa, radio, gravedad) pregunta primero MANUAL o ALEATORIO, y cuando
 * aplica (velocidad/masa/radio) también GLOBAL o POR PARTÍCULA.
 *
 * Efectos:
 *   - Asigna memoria dinámica para .velocidad.valores, .masa.valores,
 *     .radio.valores cuando el alcance es POR_PARTICULA (n ya debe estar
 *     definido antes de llamarse).
 *   - Valida rangos mínimos razonables y vuelve a preguntar si hay error.
 */
void configurar_interactivamente(ConfigSimulacion* c);

/**
 * Libera la memoria dinámica asignada por configurar_interactivamente()
 * (los arrays .valores de velocidad, masa y radio si existen).
 * Idempotente: puede llamarse aunque no se haya asignado memoria.
 */
void liberar_parametros(ConfigSimulacion* c);

/* =============================================================
 * PROTOTIPOS — fisicas.c
 * ============================================================= */

/**
 * Inicializa N partículas con masa, radio, posición y velocidad.
 * Respeta la configuración declarada en ConfigSimulacion (modo manual
 * o aleatorio, alcance global o por partícula para cada atributo).
 * Intenta evitar solapamiento inicial con hasta 2000 reintentos por partícula.
 */
void inicializar_particulas(Particula* p, int n, ConfigSimulacion* c);

/**
 * Aplica gravedad constante: vy += g * dt (eje Y positivo hacia abajo).
 */
void aplicar_gravedad(Particula* p, int n, ConfigSimulacion* c);

/**
 * Aplica fricción viscosa: vx *= (1 - μ*dt),  vy *= (1 - μ*dt).
 */
void aplicar_friccion(Particula* p, int n, ConfigSimulacion* c);

/**
 * Integración de Euler: x += vx*dt,  y += vy*dt.
 */
void mover_particulas(Particula* p, int n, double dt);

/**
 * Detecta colisión con las 4 paredes y refleja velocidad con coef. restitución.
 * Reposiciona la partícula para evitar penetración en la pared.
 */
void colision_paredes(Particula* p, int n, ConfigSimulacion* c);

/**
 * Algoritmo O(N²): evalúa N*(N-1)/2 pares únicos de partículas.
 * Si dist(p_i, p_j) < r_i + r_j → resuelve colisión elástica 2D.
 * Optimización: compara dist² vs (r1+r2)² para evitar sqrt innecesario.
 */
void detectar_y_resolver_colisiones(Particula* p, int n);

/**
 * Retorna la energía cinética total: Σ (½ * m_i * (vx_i² + vy_i²))
 */
double calcular_energia_cinetica(Particula* p, int n);

/**
 * Retorna la energía potencial gravitacional total: Σ (m_i * g * (alto - y_i))
 * Usa (alto - y) para convertir coordenada Y-abajo a altura real.
 */
double calcular_energia_potencial(Particula* p, int n, double g, double alto);

/* =============================================================
 * PROTOTIPOS — csv_export.c
 * ============================================================= */

/**
 * Crea y abre trayectorias.csv con header: tick,particula_id,x,y,vx,vy
 * Termina el programa con EXIT_FAILURE si fopen falla.
 */
FILE* abrir_csv_trayectorias(const char* nombre);

/**
 * Crea y abre energia.csv con header: tick,energia_cinetica,energia_potencial,energia_total
 */
FILE* abrir_csv_energia(const char* nombre);

/**
 * Escribe una fila por cada partícula en el tick actual → trayectorias.csv
 */
void exportar_tick_trayectorias(FILE* f, int tick, Particula* p, int n);

/**
 * Escribe una fila con las tres energías del sistema en el tick actual → energia.csv
 */
void exportar_tick_energia(FILE* f, int tick, double ek, double ep, double et);

/**
 * Registra datos de rendimiento en benchmark.csv (modo append).
 * Escribe header solo si el archivo no existía previamente.
 */
void exportar_benchmark(const char* nombre, int n, int ticks, double tiempo);

#endif /* PARTICULA_H */
