# Avance 04 — Diagrama de Casos de Uso (UML)

**Rama:** `feature/input-interactivo`
**Curso:** SW-303 Lenguaje de Programación I
**Tipo:** UML de casos de uso — cubre tanto el caso de uso de negocio como el de sistema solicitados.

---

## 1. Caso de uso de negocio

Este producto de software puede ser utilizado por ingenieros y científicos que necesiten simular la dinámica de N partículas en 2D bajo gravedad, fricción y colisiones elásticas — por ejemplo para docencia de mecánica clásica, prototipos de *granular matter*, o validación de algoritmos de colisión antes de escalarlos a motores 3D.

```mermaid
graph TB
  subgraph Negocio["Uso de negocio - Simulador N-Particulas 2D"]
    N1(Ensenar mecanica clasica en aula)
    N2(Validar algoritmos de colision elastica)
    N3(Explorar conservacion de energia)
    N4(Benchmarking de rendimiento en C)
    N5(Generar datasets para analisis posterior)
  end

  Ingeniero((Ingeniero))
  Cientifico((Cientifico))
  Docente((Docente))
  Estudiante((Estudiante))

  Docente --> N1
  Estudiante --> N1
  Estudiante --> N3
  Cientifico --> N2
  Cientifico --> N5
  Ingeniero --> N2
  Ingeniero --> N4
```

---

## 2. Caso de uso de sistema

Describe las interacciones concretas entre el usuario y el software compilado. Incluye las relaciones `<<include>>` para los pasos obligatorios que ocurren dentro de "Ejecutar simulación" y "Configurar parámetros".

```mermaid
graph TB
  subgraph Sistema["Sistema - Simulador SW-303"]
    UC1(Configurar parametros)
    UC2(Elegir modo Manual o Aleatorio)
    UC3(Elegir alcance Global o Por particula)
    UC4(Ejecutar simulacion)
    UC5(Exportar CSV de trayectorias)
    UC6(Exportar CSV de energia)
    UC7(Exportar benchmark)
    UC8(Visualizar graficos con Python)
    UC9(Compilar con make)
    UC10(Perfilar con gprof)
  end

  Usuario((Usuario))
  Analista((Analista))

  Usuario --> UC9
  Usuario --> UC1
  Usuario --> UC4
  Analista --> UC8
  Analista --> UC10

  UC1 -.->|include| UC2
  UC1 -.->|include| UC3
  UC4 -.->|include| UC5
  UC4 -.->|include| UC6
  UC4 -.->|include| UC7
```

---

## 3. Descripción unificada

El primer diagrama captura el valor de negocio: el simulador sirve a cuatro perfiles de usuario —ingenieros, científicos, docentes y estudiantes— con propósitos que van desde la enseñanza de mecánica clásica hasta el benchmarking de algoritmos de colisión, cubriendo así el uso declarado en la especificación del entregable. El segundo diagrama, de nivel de sistema, desglosa las operaciones concretas que el usuario ejecuta por consola: compilar con `make`, configurar parámetros de forma interactiva (que incluye obligatoriamente las sub-decisiones de modo y alcance), correr la simulación (que incluye la persistencia en tres archivos CSV), y pasar la pelota al analista para graficar con Python o perfilar con `gprof`. Las relaciones `<<include>>` marcadas como flechas punteadas dejan claro qué pasos son sub-pasos obligatorios dentro de un caso de uso mayor.

---

## 4. Escenario principal (texto narrativo)

**Caso de uso:** Ejecutar simulación con parámetros personalizados.
**Actor principal:** Usuario (ingeniero/estudiante).
**Precondición:** El ejecutable `simulador` ha sido compilado con `make all`.

1. El Usuario ejecuta `./simulador` desde la terminal.
2. El sistema pregunta si desea usar configuración interactiva; el Usuario responde `S`.
3. Para cada parámetro (N, gravedad, velocidad, masa, radio) el sistema pregunta `Manual o Aleatorio`; para velocidad/masa/radio también pregunta `Global o Por partícula`.
4. El Usuario responde según su caso de prueba.
5. El sistema valida rangos y, de ser necesario, vuelve a preguntar hasta obtener entrada válida.
6. El sistema ejecuta el bucle de `ticks_totales` iteraciones aplicando física en orden determinista.
7. Tras cada tick el sistema persiste estado en `trayectorias.csv` y `energia.csv`.
8. Al terminar, el sistema imprime resumen (tiempo total, partículas·tick/s, colisiones por tick), anexa una fila a `benchmark.csv` y libera toda la memoria dinámica.

**Postcondición:** Los tres archivos CSV quedan disponibles en el directorio de trabajo para ser consumidos por `visualizar.py`.

**Flujo alternativo A (saltar menú):** En el paso 2 el Usuario responde `N` y el sistema usa los valores por defecto históricos (N aleatorio, rangos originales de masa/radio/velocidad).

**Flujo de excepción E1:** Si `calloc` o `fopen` fallan, el sistema imprime el error en `stderr`, libera lo asignado y retorna `EXIT_FAILURE` sin archivos huérfanos abiertos.
