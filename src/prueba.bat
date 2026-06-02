@echo off
REM ============================================================
REM prueba.bat - Lanzador unico del Simulador de Particulas V2
REM Todo lo generado se guarda en resultados_prueba\
REM ============================================================

setlocal EnableDelayedExpansion
cd /d "%~dp0"
          
echo   @@@@+  @@@@@@@   @@@@@@@  @@@@@@@@@@    -@@@@@@@     @@@@@@@@@# @@@@@@@@@@@    @@@@@@=           
echo   @@@@+  @@@@@@@- @@@@@@@@  @@@@  @@@@@   @@@@@@@@    @@@@@+  @      @@@@@      @@@@@@@@           
echo   @@@@+  @@@@@@@@@@@@@@@@@  @@@@@@@@@@   +@@@@+@@@@   @@@@*          @@@@.     @@@@#+@@@@          
echo   @@@@+  @@@@ #@@@@@-:@@@@  @@@@@        @@@@@@@@@@   @@@@@@*@@:     @@@@.     @@@@@@@@@@          
echo   @@@@+  @@@@  @@@@@ :@@@@  @@@@        @@@@@@@@@@@@   @@@@@@@@@@    @@@@.    @@@@@@@@@@@@         
                                 
echo ============================================================
echo            SIMULADOR DE PARTICULAS
echo ============================================================
echo.

REM ------------------------------------------------------------
REM Limpieza
REM ------------------------------------------------------------

if not exist resultados_prueba mkdir resultados_prueba

for %%F in (
    trayectorias.csv
    energia.csv
    benchmark.csv
    grafico_benchmark.png
    simulacion.gif
    heatmap_densidad.png
    grafico_energia.png
) do (
    if exist "resultados_prueba\%%F" del /q "resultados_prueba\%%F"
)

REM ------------------------------------------------------------
REM Compilacion
REM ------------------------------------------------------------

echo === Compilando simulador en C ===

gcc -Wall -Wextra -O2 ^
    c_modules\Opti.c ^
    c_modules\Fisica.c ^
    c_modules\Input.c ^
    c_modules\Output.c ^
    -o c_modules\Simulador.exe -lm

if errorlevel 1 (
    echo.
    echo ERROR: Fallo la compilacion.
    pause
    exit /b 1
)

if not exist c_modules\Simulador.exe (
    echo.
    echo ERROR: No se genero Simulador.exe
    pause
    exit /b 1
)

echo OK Compilado.
echo.

REM ------------------------------------------------------------
REM Detectar Python
REM ------------------------------------------------------------

echo === Detectando Python ===

set PYTHON_CMD=

for %%V in (3.14 3.13 3.12 3.11) do (
    py -%%V -c "import numpy,pandas,matplotlib" >nul 2>&1
    if not errorlevel 1 (
        set PYTHON_CMD=py -%%V
        goto :PYTHON_OK
    )
)

echo.
echo ERROR: No se encontro Python con:
echo   - numpy
echo   - pandas
echo   - matplotlib
echo.
echo Instala:
echo   py -3 -m pip install numpy pandas matplotlib
echo.
pause
exit /b 1

:PYTHON_OK
echo OK Python detectado: %PYTHON_CMD%
echo.

REM ------------------------------------------------------------
REM Menu
REM ------------------------------------------------------------

:MENU

echo ------------------------------------------------------------
echo  Que deseas ejecutar?
echo    1. Simulacion + graficas (GIF, energia, heatmap)
echo    2. Benchmark (varios N) + grafico de rendimiento
echo    3. Ambos
echo    4. Salir
echo ------------------------------------------------------------

set /p OPCION=Opcion [1/2/3/4]:

if "%OPCION%"=="1" (
    call :SIM
    goto MENU
)

if "%OPCION%"=="2" (
    call :BENCH
    goto MENU
)

if "%OPCION%"=="3" (
    call :SIM
    call :BENCH
    goto MENU
)

if "%OPCION%"=="4" exit /b 0

echo Opcion invalida.
goto MENU

REM ============================================================
REM SIMULACION
REM ============================================================

:SIM

echo.
echo === Simulacion ===
echo.

if exist _sim_tmp rmdir /s /q _sim_tmp
mkdir _sim_tmp

copy /Y visualizar.py _sim_tmp\ >nul

pushd _sim_tmp

echo Ejecutando simulador...

..\c_modules\Simulador.exe

echo.
echo Generando graficas...
echo Usando Python: %PYTHON_CMD%

%PYTHON_CMD% visualizar.py

if errorlevel 1 (
    echo.
    echo ERROR al ejecutar visualizar.py
)

popd

for %%F in (
    trayectorias.csv
    energia.csv
    benchmark.csv
    simulacion.gif
    heatmap_densidad.png
    grafico_energia.png
) do (
    if exist "_sim_tmp\%%F" (
        move /Y "_sim_tmp\%%F" resultados_prueba\ >nul
    )
)

rmdir /s /q _sim_tmp

echo.
echo LISTO. Resultados guardados en resultados_prueba\
echo.
goto :eof

REM ============================================================
REM BENCHMARK
REM ============================================================

:BENCH

echo.
echo === Benchmark ===
echo.

if exist _bench_tmp rmdir /s /q _bench_tmp
mkdir _bench_tmp

copy /Y visualizar.py _bench_tmp\ >nul

pushd _bench_tmp

set TAMANOS=10 50 100 250 500 1000 2000

for %%N in (!TAMANOS!) do (
    echo   --- N = %%N ---
    (echo 1000& echo 1000& echo %%N& echo s& echo 1) | ..\c_modules\Simulador.exe >nul
)

echo.
echo Generando grafico benchmark...
echo Usando Python: %PYTHON_CMD%

%PYTHON_CMD% visualizar.py --solo-benchmark

if errorlevel 1 (
    echo.
    echo ERROR al ejecutar visualizar.py
)

popd

for %%F in (
    benchmark.csv
    grafico_benchmark.png
) do (
    if exist "_bench_tmp\%%F" (
        move /Y "_bench_tmp\%%F" resultados_prueba\ >nul
    )
)

rmdir /s /q _bench_tmp

echo.
echo LISTO. Benchmark guardado en resultados_prueba\
echo.
goto :eof