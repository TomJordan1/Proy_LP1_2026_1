# Instrucciones de Ejecución Rápido en Windows
Abre una terminal de PowerShell o CMD orientada en la carpeta raíz del código fuente (Proy_LP1_2026_1/src) y ejecuta el comando de automatización relativo a makefile.

## Comando Base de Ejecución:
El comando requiere tres parámetros obligatorios para configurar el entorno sin necesidad de interacción manual por teclado. A continuación, un ejemplo de uso:

```text
mingw32-make run ANCHO=3000 ALTO=1500 N_PART=200
````

🔍 Desglose de Parámetros:
##### * ANCHO: Define el límite horizontal de la caja de simulación (p. ej., 3000).

##### * ALTO: Define el límite vertical de la caja de simulación (p. ej., 1500).

##### * N_PART: Especifica el número total de partículas a simular de manera simultánea (p. ej., 200).
