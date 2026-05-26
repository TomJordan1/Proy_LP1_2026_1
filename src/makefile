# =============================================================
# Makefile
# =============================================================

CC      = gcc
CFLAGS  = -Wall -Wextra -std=c99 -O2
LDFLAGS = -lm

# Coge todos los .c en la carpeta actual
SRC     = $(wildcard c_modules/*.c)

# Crea la subcarpeta 'target' y guarda ahí el .exe
OUT_DIR = target
TARGET  = $(OUT_DIR)/simulador.exe

# Variables por defecto
ANCHO   ?= 1000
ALTO    ?= 1000
N_PART  ?= 50

# =============================================================
# REGLAS DE COMPILACIÓN
# =============================================================
all: $(OUT_DIR) $(TARGET)

$(OUT_DIR):
	if not exist $(OUT_DIR) mkdir $(OUT_DIR)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $@ $(LDFLAGS)
	@echo "OK Compilado: $(TARGET)"

# =============================================================
# RUN: Automatización completa
# =============================================================
run: all
	@echo === 1. Ejecutando Simulador en C ===
	@echo $(ANCHO) > temp_in.txt
	@echo $(ALTO) >> temp_in.txt
	@echo $(N_PART) >> temp_in.txt
	@echo s >> temp_in.txt
	@echo 1 >> temp_in.txt
	target\simulador.exe < temp_in.txt
	@del temp_in.txt 2>nul || rm -f temp_in.txt
	
	@echo === 2. Moviendo resultados CSV a $(OUT_DIR)/ ===
	@move /y trayectorias.csv $(OUT_DIR)\ >nul 2>&1 || mv -f trayectorias.csv $(OUT_DIR)/ 2>/dev/null || true
	@move /y energia.csv $(OUT_DIR)\ >nul 2>&1 || mv -f energia.csv $(OUT_DIR)/ 2>/dev/null || true
	@move /y benchmark.csv $(OUT_DIR)\ >nul 2>&1 || mv -f benchmark.csv $(OUT_DIR)/ 2>/dev/null || true
	
	@echo === 3. Renderizando animacion con Python ===
	py animador_particulas.py $(ANCHO) $(ALTO) $(OUT_DIR)/trayectorias.csv

# =============================================================
# LIMPIEZA
# =============================================================
clean:
	rm -rf $(OUT_DIR) 2>/dev/null || rmdir /s /q $(OUT_DIR)