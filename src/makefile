CC = gcc
CFLAGS = -Wall -Wextra -O2
LDFLAGS = -lm
SRC = c_modules/Opti.c c_modules/Fisica.c c_modules/Input.c c_modules/Output.c
EXEC = c_modules/Simulador.exe
PYTHON ?= python
OUT_DIR = resultados_prueba
TAMANOS = 10 50 100 250 500 1000 2000

.PHONY: all sim bench clean help check-deps

# Target por defecto
all: help

help:
	@echo ============================================================
	@echo                    SIMULADOR DE PARTICULAS                  
	@echo ============================================================
	@echo Opciones de ejecucion disponibles:
	@echo   mingw32-make sim    : Ejecuta la simulacion + graficas
	@echo   mingw32-make bench  : Ejecuta el benchmark de rendimiento
	@echo   mingw32-make all_run: Ejecuta tanto simulacion como benchmark
	@echo   mingw32-make clean  : Borra archivos temporales y exe
	@echo ============================================================

check-deps:
	@echo === Verificando dependencias de Python ===
	@$(PYTHON) -c "import numpy, pandas, matplotlib, PIL" >nul 2>&1 || (echo ERROR: Faltan librerias. && exit 1)
	@echo OK - Dependencias listas.

$(EXEC):
	@echo === Compilando simulador en C ===
	$(CC) $(CFLAGS) $(SRC) -o $(EXEC) $(LDFLAGS)
	@echo OK - Compilado exitosamente.

$(OUT_DIR):
	@mkdir $(OUT_DIR) 2>nul || (exit 0)

sim: check-deps $(EXEC) $(OUT_DIR)
	@echo === Iniciando Simulacion ===
	@rmdir /S /Q _sim_tmp 2>nul || (exit 0)
	@mkdir _sim_tmp
	@copy /Y visualizar.py _sim_tmp\ >nul
	@echo Ejecutando simulador en C...
	@cd _sim_tmp && ..\c_modules\Simulador.exe
	@echo Generando graficas con Python...
	@cd _sim_tmp && $(PYTHON) visualizar.py
	@move /Y _sim_tmp\*.csv $(OUT_DIR)\ >nul 2>&1 || (exit 0)
	@move /Y _sim_tmp\*.gif $(OUT_DIR)\ >nul 2>&1 || (exit 0)
	@move /Y _sim_tmp\*.png $(OUT_DIR)\ >nul 2>&1 || (exit 0)
	@rmdir /S /Q _sim_tmp
	@echo LISTO. Resultados guardados en $(OUT_DIR)\

bench: check-deps $(EXEC) $(OUT_DIR)
	@echo === Iniciando Benchmark ===
	@rmdir /S /Q _bench_tmp 2>nul || (exit 0)
	@mkdir _bench_tmp
	@copy /Y visualizar.py _bench_tmp\ >nul
	@cd _bench_tmp && for %%N in ($(TAMANOS)) do ( echo --- Corriendo N = %%N --- && (echo 1000^& echo 1000^& echo %%N^& echo s^& echo 1) | ..\c_modules\Simulador.exe >nul )
	@echo Generando grafico de rendimiento...
	@cd _bench_tmp && $(PYTHON) visualizar.py --solo-benchmark
	@move /Y _bench_tmp\*.csv $(OUT_DIR)\ >nul 2>&1 || (exit 0)
	@move /Y _bench_tmp\*.png $(OUT_DIR)\ >nul 2>&1 || (exit 0)
	@rmdir /S /Q _bench_tmp
	@echo LISTO. Benchmark guardado en $(OUT_DIR)\

all_run: sim bench

clean:
	@echo === Limpiando entorno ===
	@rmdir /S /Q _sim_tmp 2>nul || (exit 0)
	@rmdir /S /Q _bench_tmp 2>nul || (exit 0)
	@rmdir /S /Q $(OUT_DIR) 2>nul || (exit 0)
	@del /Q c_modules\Simulador.exe 2>nul || (exit 0)
	@echo OK - Entorno limpio.