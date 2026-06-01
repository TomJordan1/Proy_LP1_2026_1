@echo off
REM ============================================================
REM prueba.bat - Ejecuta TODO el flujo en una carpeta nueva
REM   1) Compila el simulador en C
REM   2) Crea la carpeta resultados_prueba
REM   3) Corre el simulador (responde el menu: 1000 1000 100 s 1)
REM   4) Genera las graficas (energia, heatmap, benchmark, GIF)
REM Uso: doble clic, o desde la terminal:  prueba.bat
REM ============================================================
setlocal
cd /d "%~dp0"

echo === [1/4] Compilando simulador ===
gcc -Wall -Wextra -O2 c_modules\Opti.c c_modules\Fisica.c c_modules\Input.c -o c_modules\Simulador.exe -lm
if errorlevel 1 (
    echo  Aviso: fallo la compilacion. Se intentara usar c_modules\Simulador.exe existente.
)
if not exist c_modules\Simulador.exe (
    echo  ERROR: no existe el ejecutable y no se pudo compilar. Revisa que gcc este instalado.
    pause
    exit /b 1
)

echo === [2/4] Creando carpeta de resultados ===
if not exist resultados_prueba mkdir resultados_prueba
cd resultados_prueba

echo === [3/4] Ejecutando simulador (responde: 1000 1000 100 s 1) ===
..\c_modules\Simulador.exe

echo === [4/4] Generando graficas con Python ===
python ..\visualizar.py
if errorlevel 1 (
    echo  Aviso: no se pudieron generar las graficas. Revisa que tengas instaladas las librerias:
    echo    pip install pandas matplotlib numpy pillow
)

echo.
echo === LISTO. Resultados en: src\resultados_prueba\ ===
echo   trayectorias.csv, energia.csv, benchmark.csv
echo   grafico_energia.png, heatmap_densidad.png, grafico_be