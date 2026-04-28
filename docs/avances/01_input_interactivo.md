# Entrada interactiva de parámetros

**Rama:** `feature/input-interactivo`
**Fecha:** 2026-04-19
**Curso:** SW-303 Lenguaje de Programación I
**Estado:** ✅ Implementado y probado (compila con `-Wall -Wextra -std=c99 -O2`, sin warnings)

---

## 1. Objetivo del avance

Modificar el simulador para que los siguientes parámetros dejen de ser *hard-coded* y
el usuario pueda elegirlos de forma interactiva antes de la simulación, eligiendo
en cada caso si el valor es **MANUAL** (ingresado por teclado) o **ALEATORIO**
(sorteado por `rand()` en un rango), y —para atributos por partícula— si es
**GLOBAL** (mismo valor para todas) o **POR PARTÍCULA** (uno distinto por cada una):

| Parámetro            | Manual/Aleatorio | Global/Por Partícula |
|----------------------|:----------------:|:--------------------:|
| Número de partículas | ✅               | N/A (escalar)        |
| Gravedad (m/s²)      | ✅ (default 9.81)| N/A (escalar)        |
| Velocidad (m/s)      | ✅               | ✅                   |
| Masa (kg)            | ✅               | ✅                   |
| Radio (m)            | ✅               | ✅                   |

La opción de saltar el menú (`N` al iniciar) mantiene el comportamiento histórico
del simulador (todos aleatorios, por partícula, rangos físicos originales).

---

## 2. Cambios en el código

### 2.1 Nuevos tipos en `src/particula.h`

```c
typedef enum { MODO_ALEATORIO = 0, MODO_MANUAL = 1 } ModoEntrada;
typedef enum { ALCANCE_GLOBAL = 0, ALCANCE_POR_PARTICULA = 1 } AlcanceValor;

typedef struct {
    ModoEntrada  modo;
    AlcanceValor alcance;
    double       valor_global;
    double       min, max;
    double*      valores;   // array[N] si alcance = POR_PARTICULA
} ParametroInteractivo;
```

`ConfigSimulacion` ahora incluye `ParametroInteractivo velocidad, masa, radio`
y `ModoEntrada modo_n, modo_gravedad`.

### 2.2 Nuevo módulo `src/entrada.c`

Implementa dos funciones públicas:

- `configurar_interactivamente(ConfigSimulacion* c)` — orquesta el menú en stdout.
- `liberar_parametros(ConfigSimulacion* c)` — libera arrays `.valores` dinámicos.

Internamente usa `preguntar_modo`, `preguntar_alcance`, `leer_double`, `leer_int`
y `configurar_parametro` (genérica para velocidad/masa/radio).

### 2.3 Cambios en `src/fisicas.c`

Se añadió `valor_para_particula(ParametroInteractivo*, int i)` que centraliza la
lógica de las 4 combinaciones posibles (Manual/Aleatorio × Global/PorPartícula).
`inicializar_particulas` ahora la utiliza en lugar de llamar directamente a `rand()`.

### 2.4 Cambios en `src/simulador.c`

- Llama a `configurar_interactivamente(&config)` después de los args CLI.
- Inicializa `velocidad.valores = masa.valores = radio.valores = NULL` para que
  `liberar_parametros` sea siempre seguro.
- Llama a `liberar_parametros(&config)` al final (antes de `return`).
- `imprimir_configuracion` ahora muestra los modos (MANUAL/ALEATORIO,
  GLOBAL/POR_PART) y rangos para cada parámetro.

### 2.5 Makefile

Sin cambios: el target `all` usa `wildcard src/*.c`, así que el nuevo
`src/entrada.c` se compila automáticamente.

---

## 3. Flujo del menú

```
====================================================
   CONFIGURACION INTERACTIVA DE PARAMETROS
====================================================
Usar configuracion interactiva? [S]i/[N]o:

-- Configurar NUMERO DE PARTICULAS --
  >> Numero de particulas: [M]anual o [A]leatorio?
  ... (pide valor o rango)

-- Configurar GRAVEDAD --
  >> Gravedad: [M]anual o [A]leatorio?
     (valor por defecto fisico terrestre: 9.81 m/s^2)
  ... (pide valor o rango)

-- Configurar Velocidad (m/s) --
  >> Velocidad (m/s): [M]anual o [A]leatorio?
  >> Velocidad (m/s): [G]lobal o [P]or particula?
  ... (pide valor único, N valores, o rango)

-- Configurar Masa (kg) --          (idéntico flujo)
-- Configurar Radio (m) --           (idéntico flujo)
```

---

## 4. Pruebas realizadas

| Escenario                                          | Resultado |
|----------------------------------------------------|:---------:|
| Saltar menú (`N`) → valores default                | ✅        |
| N=5 MANUAL, gravedad MANUAL=9.81, velocidad ALEATORIO/GLOBAL | ✅ |
| N=3 ALEATORIO, masa MANUAL/POR_PART (3 valores)     | ✅        |
| Compilación `-Wall -Wextra -std=c99 -O2`            | ✅ 0 warnings |

Comando de prueba:

```bash
make clean && make all
./simulador            # menú interactivo completo
./simulador 20 300      # menú interactivo, N=20 y ticks=300 como defaults del CLI
```

---

## 5. Archivos modificados/creados

```
src/particula.h          (modificado)  +70 líneas aprox
src/entrada.c            (NUEVO)       ~250 líneas
src/fisicas.c            (modificado)  refactor de inicializar_particulas
src/simulador.c          (modificado)  +30 líneas (llamadas al menú + liberación)
docs/avances/01_input_interactivo.md  (NUEVO, este archivo)
```

---

## 6. Próximos avances

Pendientes (en orden de entrega):

1. `02_diagrama_estructuras.md`   — UML de estructuras (structs y relaciones)
2. `03_diagrama_secuencia.md`     — UML de secuencia (loop de simulación)
3. `04_caso_de_uso.md`            — UML de casos de uso (negocio + sistema)
4. `05_gui.md`                    — Propuesta/implementación de GUI

---

## 7. Comandos git para esta entrega

```bash
# 1. Crear y cambiar a la rama
git checkout -b feature/input-interactivo

# 2. Stagear los archivos
git add src/particula.h src/entrada.c src/fisicas.c src/simulador.c
git add docs/avances/01_input_interactivo.md

# 3. Commit con mensaje descriptivo
git commit -m "feat(entrada): menu interactivo manual/aleatorio y global/por-particula

- Agrega enums ModoEntrada y AlcanceValor en particula.h
- Nuevo modulo src/entrada.c con configurar_interactivamente() y
  liberar_parametros()
- inicializar_particulas ahora respeta la config del menu (valor
  unico, N valores manuales, rango aleatorio global, rango aleatorio
  por particula)
- Gravedad default 9.81, permite MANUAL o ALEATORIO en [0,20] m/s^2
- Opcion 'N' al iniciar salta el menu y mantiene comportamiento previo
- Compila sin warnings con -Wall -Wextra -std=c99 -O2"

# 4. Push al remoto (crea la rama en GitHub)
git push -u origin feature/input-interactivo
```
