#include "Simulador.h"

// FUNCIONES DE INTERFAZ / CONFIGURACIÓN
Config configurar_sistema() {
    Config c;
    char opcion;
    
    printf("--- IMPACTA ---\n");
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