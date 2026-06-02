@echo off
REM ============================================================
REM prueba.bat - Lanzador unico del Simulador de Particulas V2
REM   Todo lo generado se guarda en  resultados_prueba\
REM ============================================================
setlocal enabledelayedexpansion
cd /d "%~dp0"

echo ============================================================
echo            SIMULADOR DE PARTICULAS 2D - IMPACTA
echo ============================================================
echo.

REM --- Limpieza de archivos sueltos en src\ (basura de corridas viejas) ---
for %%F in (trayectorias.csv energia.csv benchmark.csv grafico_benchmark.png simulacion.gif heatmap_densidad.png grafico_energia.png) do (
    if exist "%%F" del /q "%%F"
)

echo === Compilando simulador en C ===
gcc -Wall -Wextra -O2 c_modules\Opti.c c_modules\Fisica.c c_modules\Input.c -o c_modules\Simulador.exe -lm
if not exist c_modules\Simulador.exe (
    echo  ERROR: no se pudo compilar. Verifica que gcc este instalado.
    pause & exit /b 1
)
echo  OK Compilado.
if not exist resultados_prueba mkdir resultados_prueba
echo.

:MENU
echo ------------------------------------------------------------
echo  Que deseas ejecutar?
echo    1. Simulacion + graficas (GIF, energia, heatmap)
echo    2. Benchmark (varios N) + grafico de rendimiento
echo    3. Ambos (simulacion y luego benchmark)
echo    4. Salir
echo ------------------------------------------------------------
set /p OPCION=Opcion [1/2/3/4]: 

if "%OPCION%"=="1" ( call :SIM & goto MENU )
if "%OPCION%"=="2" ( call :BENCH & goto MENU )
if "%OPCION%"=="3" ( call :SIM & call :BENCH & goto MENU )
if "%OPCION%"=="4" exit /b 0
echo Opcion invalida.
goto MENU

REM ---------------- SUBRUTINA: SIMULACION ----------------
:SIM
echo.
echo === Simulacion: responde el menu (ej: 1000 1000 100 s 1) ===
pushd resultados_prueba
..\c_modules\Simulador.exe
echo === Generando graficas ===
python ..\visualizar.py
popd
echo.
echo  LISTO. Resultados en src\resultados_prueba\
echo.
goto :eof

REM ---------------- SUBRUTINA: BENCHMARK ----------------
REM  Corre en una carpeta temporal para no dejar CSV sueltos.
REM  Solo benchmark.csv y grafico_benchmark.png quedan en resultados_prueba.
:BENCH
echo.
echo === Benchmark: corriendo varios tamanos de N ===
if exist _bench_tmp rmdir /s /q _bench_tmp
mkdir _bench_tmp
pushd _bench_tmp
set TAMANOS=10 50 100 250 500 1000 2000
for %%N in (!TAMANOS!) do (
    echo   --- N = %%N ---
    (echo 1000& echo 1000& echo %%N& echo s& echo 1) | ..\c_modules\Simulador.exe >nul
)
echo === Generando grafico de benchmark ===
python ..\visualizar.py --solo-benchmark
move /y benchmark.csv ..\resultados_prueba\ >nul
move /y grafico_benchmark.png ..\resultados_prueba\ >nul
popd
rmdir /s /q _bench_tmp
echo.
echo  LISTO. benchmark.csv y grafico_benchmark.png en src\resultados_prueba\
echo.
goto :eof
