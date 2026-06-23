<h1 align="center">
  <strong>IMPACTA</strong>
</h1>

<p align="center">
  Simulador de partículas 2D con física básica
</p>


<p align="center">
  <img src="example.gif" width="250" alt="Ejemplo de gif de salida">
</p>

![Python](https://img.shields.io/badge/python-3670A0?style=for-the-badge&logo=python&logoColor=ffdd54) ![C](https://img.shields.io/badge/c-%2300599C.svg?style=for-the-badge&logo=c&logoColor=white)

El presente proyecto describe el diseño e implementación de un simulador de colisiones de partículas en 2D, estructurado como una plataforma dual para la física computacional y la generación estructurada de datos, idónea para *Machine Learning*.

## 👥 Roles
* **QA, Testing & DevOps:** Tom Jordan Pérez López  
* **Developer 1 (C):** Ricardo Gaspar Reyes Campos
* **Apoyo transversal:** Sandro Giancarlo Sanchez Vicaña
* **Developer 2 (Python):** Raúl Antonio Guzmán López

## 📎 Recursos
* **Diapositivas (Canva):** [Presentación del Proyecto](https://canva.link/zb432bjr0ofj4bi)

## 📂 Estructura del Repositorio

```text
Proy_LP1_2026_1/
├── .gitignore                  # Archivos y carpetas ignorados por Git
├── example.gif                 # Ejemplo de la salida deseada
├── README.md                   # Documentación principal del proyecto
├── guia_de_uso_rapido.md       # Manual conciso para probar el programa del proyecto
├── docs/                       # Documentación adicional, informes o manuales del proyecto
└── src/                        # Código fuente principal
    ├── c_modules/              # Módulos del simulador en C
    │   ├── Fisica.c            # Lógica de físicas, movimientos y colisiones
    │   ├── Input.c             # Manejo de menús y entrada de datos del usuario
    |   ├── Output.c            # Manejo de los archivos de salida de la simulación
    │   ├── Opti.c              # Archivo principal (Main) y optimización espacial
    │   └── Simulador.h         # Cabecera central (estructuras, prototipos y librerías)
    ├── visualizar.py           # Script de Python para renderizar la simulación
    ├── run.bat                 # Script ejecutable para probar el programa (compilación, ejecución y renderizado)
    ├── requirements.txt        # Archivo de texto con las depedencias para el módulo en Python
    └── makefile                # Script de automatización (compilación, ejecución y renderizado)
