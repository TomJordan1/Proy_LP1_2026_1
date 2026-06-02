@echo off
REM ============================================================
REM librerias_python.bat
REM Instala las librerias de Python necesarias para el simulador:
REM   - visualizar.py        : pandas, matplotlib, numpy, pillow
REM   - visualizar_realtime.py: pyqtgraph, PyQt5, imageio, imageio-ffmpeg
REM Uso: doble clic, o en terminal:  librerias_python.bat
REM ============================================================
setlocal

echo === Verificando Python ===
python --version >nul 2>&1
if errorlevel 1 (
    echo  ERROR: No se encontro "python" en el PATH.
    echo  Instala Python 3 desde https://www.python.org y marca "Add to PATH".
    pause
    exit /b 1
)
python --version

echo.
echo === Actualizando pip ===
python -m pip install --upgrade pip

echo.
echo === Instalando librerias ===
python -m pip install pandas matplotlib numpy pillow pyqtgraph PyQt5 imageio imageio-ffmpeg
if errorlevel 1 (
    echo.
    echo  Aviso: alguna libreria fallo. Revisa tu conexion e intenta de nuevo.
    pause
    exit /b 1
)

echo.
echo === LISTO ===
echo Librerias instaladas correctamente.
echo   Graficas/GIF : python visualizar.py
echo   Visor en vivo: python visualizar_realtime.py trayectorias.csv
echo.
pause
