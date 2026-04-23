# Avance 02 — Diagrama de Estructuras (UML)

**Rama:** `feature/input-interactivo`
**Curso:** SW-303 Lenguaje de Programación I
**Tipo:** UML de estructuras (estático) — representado como *class diagram* en Mermaid

---

## 1. Diagrama

```mermaid
classDiagram
  direction LR

  class Particula {
    +double x
    +double y
    +double vx
    +double vy
    +double masa
    +double radio
  }

  class ConfigSimulacion {
    +int n_particulas
    +ModoEntrada modo_n
    +double gravedad
    +ModoEntrada modo_gravedad
    +double friccion
    +double dt
    +int ticks_totales
    +double ancho
    +double alto
    +double coef_restitucion
    +ParametroInteractivo velocidad
    +ParametroInteractivo masa
    +ParametroInteractivo radio
  }

  class ParametroInteractivo {
    +ModoEntrada modo
    +AlcanceValor alcance
    +double valor_global
    +double min
    +double max
    +double* valores
  }

  class ModoEntrada {
    <<enumeration>>
    MODO_ALEATORIO
    MODO_MANUAL
  }

  class AlcanceValor {
    <<enumeration>>
    ALCANCE_GLOBAL
    ALCANCE_POR_PARTICULA
  }

  class ModuloEntrada {
    <<modulo>>
    configurar_interactivamente()
    liberar_parametros()
  }

  class ModuloFisicas {
    <<modulo>>
    inicializar_particulas()
    aplicar_gravedad()
    aplicar_friccion()
    mover_particulas()
    colision_paredes()
    detectar_y_resolver_colisiones()
    calcular_energia_cinetica()
    calcular_energia_potencial()
  }

  class ModuloCSVExport {
    <<modulo>>
    abrir_csv_trayectorias()
    abrir_csv_energia()
    exportar_tick_trayectorias()
    exportar_tick_energia()
    exportar_benchmark()
  }

  class Simulador {
    <<main>>
    main()
    imprimir_configuracion()
  }

  ConfigSimulacion "1" *-- "3" ParametroInteractivo : compone
  ParametroInteractivo ..> ModoEntrada : usa
  ParametroInteractivo ..> AlcanceValor : usa
  ConfigSimulacion ..> ModoEntrada : usa
  ModuloEntrada ..> ConfigSimulacion : configura
  ModuloFisicas ..> Particula : opera sobre
  ModuloFisicas ..> ConfigSimulacion : consulta
  ModuloCSVExport ..> Particula : serializa
  Simulador ..> ModuloEntrada : invoca
  Simulador ..> ModuloFisicas : invoca
  Simulador ..> ModuloCSVExport : invoca
  Simulador "1" o-- "N" Particula : gestiona
```

---

## 2. Descripción

El diagrama representa la organización estática del simulador de N partículas en 2D. Las tres estructuras de datos (`Particula`, `ConfigSimulacion`, `ParametroInteractivo`) y las dos enumeraciones (`ModoEntrada`, `AlcanceValor`) forman el núcleo del modelo; sobre ellas actúan cuatro módulos funcionales implementados en archivos `.c` independientes: `entrada`, `fisicas`, `csv_export` y el `simulador` principal que orquesta el flujo. La composición `ConfigSimulacion *-- ParametroInteractivo` expresa que la configuración contiene tres instancias empotradas (velocidad, masa, radio), mientras que las relaciones de dependencia (`..>`) indican que los módulos consultan o modifican los datos sin poseerlos. La agregación `Simulador o-- Particula` modela la reserva dinámica del arreglo de partículas mediante `calloc`, cuya memoria se libera al finalizar.

---

## 3. Observaciones de diseño

- **Cohesión por módulo**: cada archivo `.c` agrupa funciones con responsabilidad única (entrada interactiva / física / persistencia CSV / orquestación).
- **Acoplamiento bajo**: los módulos se comunican a través de `ConfigSimulacion` y el puntero a `Particula*`, no por variables globales.
- **Enumeraciones tipadas**: `ModoEntrada` y `AlcanceValor` evitan *magic numbers* (0/1) en el código cliente y documentan la intención.
- **Extensibilidad**: agregar un nuevo atributo por partícula (ej. color, carga eléctrica) implica sumar un `ParametroInteractivo` a `ConfigSimulacion` sin tocar los módulos de física o CSV.
