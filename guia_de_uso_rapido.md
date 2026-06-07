# makefile

## Comando Base de Ejecución

Abre una terminal de PowerShell o CMD orientada en la carpeta raíz del código fuente (Proy_LP1_2026_1/src) y ejecuta el comando de automatización relativo a makefile.

```text
mingw32-make 
```

Después de ello, se mostrará esta TUI con opciones a elegir para probar el programa.

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

Independientemente de la opción escogida, se verificará si el computador tiene las dependencias necesarias para correr los módulos en C y Python. En caso que no, los instalará en la última versión de Python al día de hoy, que para este caso es la 3.14.

### Módulo C

Luego se te pedirá ingresar las dimensiones del entorno que contendrá a las n partículas, que también se solicitará indicar dicha cantidad n.

Subsiguiente a ello, deberá indicar si se correrá la simulación en condiciones físicas ideales (rozamiento con paredes igual a cero y coeficiente de restitución igual a 1) o personalizadas.

Por último, se preguntará si se desea usar masas y radios aleatorios para cada una de las n partículas (dentro de ciertos parámetros internos) o ingresar manualmente uno por uno.

### Módulo Python

Previo a la generación de las imágenes solicitadas al principio, se consultará si se desea agregar más información sobre las partículas dentro de la animación contenida en simulacion.gif. De acuerdo a lo que se requiera, deberá elegir una de las 3 opciones.

```text
Como quieres la animacion?
1. Simple        - circulos por radio (mas rapido)
2. Con flechas   - circulos + vector de velocidad
3. Flechas + radio - circulos + flechas + tamano del radio
```

En caso de haber solicitado realizar el benchmark, se probará para los casos donde n sea igual a 10, 50, 100, 250, 500, 1000, 2000.

# bat

Los pasos para este ejecutable son equivalentes a los ya vistos en el makefile, solo que al momento de correrlo, le saldrá en pantalla lo siguiente:

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

Según lo que se requiera, se selecciona alguna de las opciones y el resto del flujo de uso sería el que ya se describió.
