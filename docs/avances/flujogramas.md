# Flujogramas — Simulador de N Partículas en 2D

**Curso:** SW-303 · Lenguajes de Programación I (Imperativo)
**Proyecto:** Simulador de N partículas con colisiones elásticas, gravedad y fricción
**Stack:** C (C99/C11) · Python/matplotlib · Makefile · gprof
**Autor:** raul · **Fecha:** 2026-04-18

> Los diagramas están escritos en **Mermaid**. Se renderizan automáticamente en GitHub, GitLab, VS Code (extensión *Markdown Preview Mermaid Support*) y en la mayoría de visores modernos de Markdown.

---

## 1. Flujograma global del proceso completo

Vista end-to-end desde la compilación hasta los gráficos finales. Muestra las tres grandes fases del proyecto: **compilación**, **simulación** y **visualización**.

```mermaid
flowchart TD
    A([Inicio]) --> B[make all<br/>Compilar con gcc -O2 -Wall -Wextra -lm]
    B --> C{¿Compilación<br/>exitosa?}
    C -- No --> C1[Revisar errores<br/>de compilación] --> B
    C -- Sí --> D[Ejecutar ./simulador N ticks]
    D --> E[Fase C: Simulación física<br/>loop de ticks]
    E --> F[Generar archivos CSV:<br/>trayectorias · energía · benchmark]
    F --> G[python visualizar.py]
    G --> H[Leer CSV con pandas]
    H --> I[Generar gráficos matplotlib:<br/>animación · energía · heatmap · benchmark]
    I --> J[Guardar PNG dpi=150<br/>y GIF de animación]
    J --> K([Fin])

    style A fill:#d4f4dd,stroke:#2e7d32
    style K fill:#d4f4dd,stroke:#2e7d32
    style E fill:#fff3cd,stroke:#f57f17
    style I fill:#e3f2fd,stroke:#1565c0
```

---

## 2. Flujograma del `main()` — Loop principal de simulación

Corresponde al orden de operaciones documentado en `src/simulador.c`. La secuencia por tick está justificada en el Entregable 1: **gravedad → fricción → movimiento → paredes → colisiones → energía → CSV**.

```mermaid
flowchart TD
    A([main inicia]) --> B[srand time<br/>Iniciar semilla aleatoria]
    B --> C[clock inicio<br/>Iniciar temporizador]
    C --> D[Cargar ConfigSimulacion<br/>con valores por defecto]
    D --> E{"¿argc &gt;= 2?"}
    E -- Sí --> F[Parsear N y ticks<br/>desde argv]
    E -- No --> G[Usar defaults:<br/>N=50, ticks=500]
    F --> H[imprimir_configuracion]
    G --> H
    H --> I[calloc Particula * N]
    I --> J{¿memoria<br/>asignada?}
    J -- No --> J1[Error: abortar<br/>EXIT_FAILURE] --> Z([Fin])
    J -- Sí --> K[inicializar_particulas<br/>posición y velocidad aleatoria]
    K --> L[abrir_csv_trayectorias<br/>abrir_csv_energia]
    L --> M{¿archivos<br/>abiertos?}
    M -- No --> M1[Error I/O: liberar<br/>y abortar] --> Z
    M -- Sí --> N[tick = 0]
    N --> O{"tick &lt; ticks_totales?"}
    O -- Sí --> P[Ejecutar 7 pasos<br/>del tick — ver §3]
    P --> Q[tick = tick + 1]
    Q --> O
    O -- No --> R[fclose archivos CSV]
    R --> S[Calcular tiempo_total<br/>y tasa part·ticks/s]
    S --> T[exportar_benchmark<br/>modo append]
    T --> U[Imprimir resumen<br/>en consola]
    U --> V[free particulas<br/>puntero = NULL]
    V --> Z

    style A fill:#d4f4dd,stroke:#2e7d32
    style Z fill:#d4f4dd,stroke:#2e7d32
    style P fill:#fff3cd,stroke:#f57f17
    style J1 fill:#ffcdd2,stroke:#c62828
    style M1 fill:#ffcdd2,stroke:#c62828
```

---

## 3. Flujograma de actualización física por tick

Detalle de los **7 pasos** que se ejecutan dentro del `for` principal. Cada paso recorre el arreglo de partículas una vez.

```mermaid
flowchart TD
    A([Inicio del tick]) --> B[PASO 1: Gravedad<br/>vy += g · dt]
    B --> C[PASO 2: Fricción viscosa<br/>vx *= 1 - μ·dt<br/>vy *= 1 - μ·dt]
    C --> D[PASO 3: Movimiento Euler<br/>x += vx · dt<br/>y += vy · dt]
    D --> E[PASO 4: Colisión con paredes<br/>reflexión · coef. restitución]
    E --> F[PASO 5: Colisiones entre pares<br/>algoritmo O N² — ver §4]
    F --> G[PASO 6: Calcular energías<br/>Ek = ½ m v²<br/>Ep = m g y<br/>Et = Ek + Ep]
    G --> H[PASO 7: Exportar a CSV<br/>trayectorias + energía]
    H --> I{¿tick múltiplo<br/>de 100?}
    I -- Sí --> J[Imprimir progreso<br/>en consola]
    I -- No --> K([Fin del tick])
    J --> K

    style A fill:#d4f4dd,stroke:#2e7d32
    style K fill:#d4f4dd,stroke:#2e7d32
    style B fill:#fff3cd,stroke:#f57f17
    style C fill:#fff3cd,stroke:#f57f17
    style D fill:#fff3cd,stroke:#f57f17
    style E fill:#ffe0b2,stroke:#ef6c00
    style F fill:#ffcdd2,stroke:#c62828
    style G fill:#e1bee7,stroke:#6a1b9a
    style H fill:#e3f2fd,stroke:#1565c0
```

### 3.1 Subflujo — Colisión con paredes (PASO 4)

```mermaid
flowchart TD
    A([Para cada partícula i]) --> B{"¿x - r &lt; 0?"}
    B -- Sí --> B1[x = r<br/>vx = -vx · coef_rest]
    B -- No --> C{"¿x + r &gt; ancho?"}
    B1 --> C
    C -- Sí --> C1[x = ancho - r<br/>vx = -vx · coef_rest]
    C -- No --> D{"¿y - r &lt; 0?"}
    C1 --> D
    D -- Sí --> D1[y = r<br/>vy = -vy · coef_rest]
    D -- No --> E{"¿y + r &gt; alto?"}
    D1 --> E
    E -- Sí --> E1[y = alto - r<br/>vy = -vy · coef_rest]
    E -- No --> F([Siguiente partícula])
    E1 --> F

    style A fill:#d4f4dd,stroke:#2e7d32
    style F fill:#d4f4dd,stroke:#2e7d32
```

---

## 4. Flujograma de detección de colisiones O(N²)

Doble bucle anidado sobre todos los pares únicos `(i, j)` con `j > i`. Cuando dos partículas se superponen, se aplica la **respuesta elástica 2D**: descomposición en normal/tangencial, intercambio del componente normal y corrección de overlap.

```mermaid
flowchart TD
    A([detectar_y_resolver_colisiones]) --> B[i = 0]
    B --> C{"i &lt; N - 1?"}
    C -- No --> Z([Fin — todos los<br/>pares evaluados])
    C -- Sí --> D[j = i + 1]
    D --> E{"j &lt; N?"}
    E -- No --> E2[i = i + 1] --> C
    E -- Sí --> F[Calcular dx, dy<br/>entre centros i y j]
    F --> G[d = sqrt dx² + dy²<br/>sumaR = ri + rj]
    G --> H{"¿d &lt; sumaR?"}
    H -- No --> H2[j = j + 1] --> E
    H -- Sí --> I[Calcular vector<br/>normal unitario nx, ny]
    I --> J[Proyectar velocidades<br/>sobre la normal:<br/>v1n · v2n]
    J --> K[Aplicar colisión elástica 1D<br/>con conservación de<br/>momento y energía]
    K --> L[Recomponer velocidades<br/>vx, vy con nueva normal<br/>+ tangencial intacta]
    L --> M[Corrección de overlap:<br/>separar partículas<br/>según sus masas]
    M --> H2

    style A fill:#d4f4dd,stroke:#2e7d32
    style Z fill:#d4f4dd,stroke:#2e7d32
    style H fill:#fff3cd,stroke:#f57f17
    style K fill:#ffcdd2,stroke:#c62828
    style M fill:#e1bee7,stroke:#6a1b9a
```

**Complejidad:** el algoritmo evalúa `N·(N-1)/2` pares por tick → **O(N²)**. Para `N=50` son 1 225 comparaciones/tick; para `N=500` son 124 750 comparaciones/tick.

**Optimización futura sugerida:** *spatial hashing* o *uniform grid* para reducir a **O(N)** amortizado al comparar únicamente partículas en celdas vecinas.

---

## 5. Flujograma del pipeline C → CSV → Python

Flujo de datos entre los dos lenguajes. El simulador en C produce CSVs planos que Python consume con pandas para producir los gráficos.

```mermaid
flowchart LR
    subgraph C [Fase C · Simulador nativo]
        C1[simulador.c<br/>loop principal] --> C2[fisicas.c<br/>física + colisiones]
        C2 --> C3[csv_export.c<br/>fprintf estado]
    end

    subgraph CSV [Archivos CSV intermedios]
        D1[trayectorias.csv<br/>tick,id,x,y,vx,vy]
        D2[energia.csv<br/>tick,Ek,Ep,Et]
        D3[benchmark.csv<br/>N,ticks,tiempo,rate]
    end

    subgraph PY [Fase Python · Visualización]
        P1[pandas.read_csv] --> P2[matplotlib<br/>FuncAnimation]
        P1 --> P3[matplotlib<br/>Ek · Ep · Et vs t]
        P1 --> P4[numpy histogram2d<br/>heatmap densidad]
        P1 --> P5[matplotlib<br/>N vs rendimiento]
    end

    subgraph OUT [Salidas finales]
        O1[simulacion.gif]
        O2[grafico_energia.png]
        O3[heatmap_densidad.png]
        O4[grafico_benchmark.png]
    end

    C3 --> D1
    C3 --> D2
    C3 --> D3
    D1 --> P1
    D2 --> P1
    D3 --> P1
    P2 --> O1
    P3 --> O2
    P4 --> O3
    P5 --> O4

    style C fill:#fff3cd,stroke:#f57f17
    style CSV fill:#e3f2fd,stroke:#1565c0
    style PY fill:#d4f4dd,stroke:#2e7d32
    style OUT fill:#e1bee7,stroke:#6a1b9a
```

---

## 6. Resumen visual — Arquitectura de módulos

```mermaid
flowchart TB
    H[particula.h<br/>structs + prototipos]
    S[simulador.c<br/>main + loop]
    F[fisicas.c<br/>gravedad, fricción,<br/>movimiento, colisiones]
    E[csv_export.c<br/>I/O de CSVs]
    M[Makefile<br/>all · clean · profile · optimized]
    V[visualizar.py<br/>pandas + matplotlib]

    H -.incluido por.-> S
    H -.incluido por.-> F
    H -.incluido por.-> E
    S --> F
    S --> E
    M --> S
    S -. genera CSV .-> V

    style H fill:#fff9c4,stroke:#f57f17
    style S fill:#ffe0b2,stroke:#ef6c00
    style F fill:#ffcdd2,stroke:#c62828
    style E fill:#e3f2fd,stroke:#1565c0
    style M fill:#d1c4e9,stroke:#4527a0
    style V fill:#d4f4dd,stroke:#2e7d32
```

---

## Leyenda de colores

| Color | Significado |
|-------|-------------|
| Verde | Inicio / fin / salida exitosa |
| Amarillo | Operación física / cálculo |
| Naranja | Condición / interacción con paredes |
| Rojo | Colisión entre partículas / error |
| Morado | Energía / post-procesamiento |
| Azul | I/O · CSV · matplotlib |

---

*Fin del documento de flujogramas.*
