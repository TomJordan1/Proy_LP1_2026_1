# Avance 03 — Diagrama de Secuencia (UML)

**Rama:** `feature/input-interactivo`
**Curso:** SW-303 Lenguaje de Programación I
**Tipo:** UML de secuencia — representa una ejecución típica del simulador con configuración interactiva.

---

## 1. Diagrama

```mermaid
sequenceDiagram
  actor U as Usuario
  participant M as main (simulador.c)
  participant E as Modulo Entrada
  participant F as Modulo Fisicas
  participant C as Modulo CSV
  participant FS as Sistema de Archivos

  U->>M: ./simulador [N] [ticks]
  M->>M: srand(time(NULL))
  M->>M: inicializar ConfigSimulacion con defaults
  M->>E: configurar_interactivamente(&config)

  alt Usuario responde "S"
    E->>U: prompts (N, gravedad, velocidad, masa, radio)
    U-->>E: modo (M/A), alcance (G/P), valores o rangos
    E->>E: asignar arrays dinamicos si POR_PARTICULA
  else Usuario responde "N"
    E->>E: usar defaults historicos
  end
  E-->>M: config completamente poblada

  M->>M: calloc(N, sizeof(Particula))
  M->>F: inicializar_particulas(p, N, &config)
  F->>F: sortear/leer masa, radio, velocidad y posicion sin solapamiento
  F-->>M: particulas listas

  M->>C: abrir_csv_trayectorias()
  M->>C: abrir_csv_energia()
  C->>FS: fopen(..., "w")
  FS-->>C: FILE*
  C-->>M: handles de archivo

  loop Por cada tick (0 .. ticks_totales-1)
    M->>F: aplicar_gravedad()
    M->>F: aplicar_friccion()
    M->>F: mover_particulas()
    M->>F: colision_paredes()
    M->>F: detectar_y_resolver_colisiones()
    M->>F: calcular_energia_cinetica()
    M->>F: calcular_energia_potencial()
    F-->>M: Ek, Ep
    M->>C: exportar_tick_trayectorias()
    M->>C: exportar_tick_energia()
    C->>FS: fprintf(...)
  end

  M->>C: exportar_benchmark(N, ticks, tiempo)
  C->>FS: append a benchmark.csv
  M->>M: free(particulas)
  M->>E: liberar_parametros(&config)
  M-->>U: resumen y codigo de salida 0
```

---

## 2. Descripción

El diagrama muestra el flujo completo de una ejecución del simulador en el eje temporal. La interacción arranca con el `main` recibiendo argumentos del CLI, continúa con el nuevo módulo `Entrada` que dialoga con el usuario para poblar `ConfigSimulacion`, y prosigue con la inicialización de partículas a cargo del módulo `Fisicas`. El núcleo de la ejecución es el bucle de ticks en el cual el orquestador invoca —en el orden justificado en el Entregable 1— gravedad, fricción, movimiento, colisiones con paredes, colisiones entre partículas, cómputo de energías y persistencia en CSV. Al terminar, el `main` libera en orden los recursos: el arreglo de partículas y luego los arreglos internos de `ConfigSimulacion`, garantizando ausencia de *memory leaks*.

---

## 3. Observaciones de diseño

- **Separación de responsabilidades**: `Entrada` nunca accede a archivos CSV ni aplica física; `Fisicas` nunca hace I/O con stdin ni stdout fuera de advertencias.
- **Idempotencia de `liberar_parametros`**: puede llamarse incluso si el usuario saltó el menú, porque los punteros `.valores` se inicializan a `NULL` en `main` antes de cualquier otra operación.
- **Reversión segura ante errores**: las rutas de error (malloc fallido, fopen fallido) imprimen por `stderr` y retornan `EXIT_FAILURE`, sin dejar archivos huérfanos abiertos.
- **Escalabilidad del bucle**: el único punto cuadrático en N es `detectar_y_resolver_colisiones`; el resto es lineal, lo que se confirma en el benchmark del Entregable 1.
