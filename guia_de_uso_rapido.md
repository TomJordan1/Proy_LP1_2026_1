# Guía de Uso Rápido - IMPACTA

Sigue estas instrucciones para ejecutar la simulación de partículas de forma sencilla en tu entorno preferido.

## ⚠️ Requisitos Previos

Antes de comenzar, asegúrate de tener instalados en tu sistema:
- **Un compilador de C** (como GCC a través de MinGW en Windows o el paquete build-essential en Linux).
- **Python 3.x** (se recomienda la versión más reciente).
- **Make** (opcional, si decides usar la opción de Makefile, como `mingw32-make` en Windows).

---

## 🚀 Opciones de Ejecución

Este programa está diseñado para automatizar el proceso de compilación, instalación de dependencias y ejecución del programa. Puedes elegir entre usar **Makefile** o un archivo **Batch (.bat)** para Windows.

### Opción 1: Usando Makefile

Abre una terminal (PowerShell, CMD o bash) orientada en la carpeta raíz del código fuente (`Proy_LP1_2026_1/src`) y ejecuta el comando base:

```text
mingw32-make 
```
*(Nota: si estás en Linux, simplemente usa `make`)*

Después de ello, se mostrará esta interfaz de texto (TUI) con las opciones disponibles:

```text
 ============================================================
                    SIMULADOR DE PARTICULAS                  
 ============================================================
 Opciones de ejecucion disponibles:
   mingw32-make sim    : Ejecuta la simulacion + graficas
   mingw32-make bench  : Ejecuta el benchmark de rendimiento
   mingw32-make all_run: Ejecuta tanto simulacion como benchmark
   mingw32-make clean  : Borra archivos temporales y exe
 ============================================================
```

### Opción 2: Usando Batch (.bat)

Esta opción es preferible si estás en Windows. Ejecuta el archivo `run.bat` desde la terminal o haciendo doble clic sobre él en la carpeta `src`. En la pantalla se mostrará lo siguiente:

```text
------------------------------------------------------------
 Que deseas ejecutar?
   1. Simulacion + graficas (GIF, energia, heatmap)
   2. Benchmark (varios N) + grafico de rendimiento
   3. Ambos
   4. Salir
------------------------------------------------------------
Opcion [1/2/3/4]:
```

Selecciona la opción deseada según lo que requieras probar.

---

## ⚙️ Flujo de Configuración

Independientemente de la opción de ejecución escogida, el sistema verificará automáticamente si tu entorno de Python cuenta con las dependencias necesarias y las instalará si faltan (`pandas`, `matplotlib`, etc.).

Tras esto, comenzará el flujo interactivo:

### 1. Configuración del Módulo C (Física)
- **Dimensiones y Cantidad:** Se te pedirá ingresar las dimensiones del entorno (caja) y la cantidad $n$ de partículas a simular.
- **Condiciones Físicas:** Deberás indicar si la simulación se correrá en condiciones físicas **ideales** (rozamiento con paredes igual a cero y coeficiente de restitución igual a 1) o **personalizadas**.
- **Propiedades de las Partículas:** Se preguntará si deseas asignar masas y radios aleatorios para cada partícula o ingresarlos manualmente uno por uno.

### 2. Configuración del Módulo Python (Visualización)
Si solicitaste ejecutar la simulación gráfica, el programa consultará el nivel de detalle deseado para el renderizado final de la animación (`simulacion.gif`).

```text
Como quieres la animacion?
1. Simple          - circulos por radio (mas rapido)
2. Con flechas     - circulos + vector de velocidad
3. Flechas + radio - circulos + flechas + tamano del radio
```

### 3. Benchmark (Opcional)
En caso de haber seleccionado una opción que incluya el benchmark de rendimiento, el sistema probará la simulación automáticamente para $n = 10, 50, 100, 250, 500, 1000$ y $2000$, generando una gráfica con los tiempos de ejecución.

---

## 🛠️ Resolución de Problemas Comunes

- **Error: `mingw32-make` no se reconoce como un comando interno o externo.**
  *Solución:* Asegúrate de tener MinGW instalado y que la ruta a la carpeta `bin` esté agregada a las Variables de Entorno (`PATH`) de tu sistema.
- **Error: `python` o `pip` no encontrado.**
  *Solución:* Durante la instalación de Python, asegúrate de marcar la casilla *"Add Python to PATH"*. Reinicia tu terminal tras instalarlo.
- **El renderizado en Python tarda demasiado.**
  *Solución:* Para grandes cantidades de partículas (ej. $n > 500$), selecciona la opción de animación **1. Simple** y limita el tiempo de simulación, ya que el trazado gráfico de `matplotlib` es costoso.
