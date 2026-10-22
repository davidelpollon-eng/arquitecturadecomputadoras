# Examen Parcial 1 — Semáforo con máquina de estados en ATmega328P

**Alumno:** David Muñoz Barrón
**Materia:** Arquitectura de Computadoras — Dr. Rubén Hernández Torres
**Carrera:** Ingeniería en Sistemas Computacionales, UAA

Control de tráfico de un crucero de dos vías con paso peatonal, implementado como una
máquina de estados finitos (6 estados) en ensamblador AVR. La tabla de estados, las salidas
y los tiempos de cada modo están en la memoria Flash y se leen con `lpm`.

La carpeta sigue la estructura de `proyecto/` del repositorio del curso.

## Estructura

| Carpeta / archivo | Qué es |
|---|---|
| `src/main.S` | Programa en ensamblador GNU para AVR |
| `makefile` | Makefile del curso, configurado con `SRC := src/main.S` |
| `release/main.hex` | Programa compilado (`-Os`), listo para grabar con `make flash` |
| `release/main.elf` | Mismo programa en formato ELF |
| `build/main.elf` | Compilación de depuración (`-ggdb3 -O0`) para simavr + GDB |
| `debug/launch_debug.sh` | Script del curso para depurar con simavr y gdbgui |
| `debug/INSTRUCTIONS.md` | Guía de depuración del curso |
| `debug/sim_semaforo.c` | Banco de prueba en simavr: emula el DIP y registra los LEDs |
| `docs/Semaforo_FSM_ATmega328P_IEEE.pdf` | Documentación en formato IEEE |
| `docs/Semaforo_FSM_ATmega328P_IEEE.docx` | La misma documentación en Word, editable |
| `docs/figuras/` | Diagrama de estados, diagrama de tiempos y diagramas de conexión |
| `docs/simulacion/` | Cambios de PORTD registrados en los 3 modos (con solicitud de cruce) |

## Conexiones

| Señal | Pin del ATmega328P |
|---|---|
| LED vía A verde / rojo | PD2 (4) / PD3 (5) |
| LED vía B verde / rojo | PD4 (6) / PD5 (11) |
| LED peatón verde / rojo | PD6 (12) / PD7 (13) |
| DIP 1: solicitud de cruce | PB0 (14) |
| DIP 2: modo bit 0 | PB1 (15) |
| DIP 3: modo bit 1 | PB2 (16) |

Cada LED lleva una resistencia de 220 Ω a GND. Los switches van a GND (se usan los pull-ups internos).

| DIP 3 | DIP 2 | Modo | Tiempo por estado |
|---|---|---|---|
| OFF | OFF | Normal | 1 s |
| OFF | ON | Hora pico | 500 ms |
| ON | OFF | Bajo tráfico | 2 s |

## Compilar, grabar y depurar

Desde esta carpeta:

```bash
make            # release/main.elf y release/main.hex
make flash      # graba con el USBasp
make debug      # build/main.elf para depurar
./debug/launch_debug.sh   # simavr + gdbgui (ver debug/INSTRUCTIONS.md)
```

El retardo está calculado para un reloj de 16 MHz (constante `3999` en `DELAY_MS`).

> En `launch_debug.sh` se cambió `make all` por `make debug`, porque el script abre
> `build/main.elf` y `make all` solo genera la versión de `release/`.

## Simular con el banco de prueba

```bash
gcc debug/sim_semaforo.c -o debug/sim_semaforo -lsimavr -lelf
./debug/sim_semaforo release/main.elf 1 1 4000
```

Argumentos: archivo ELF, modo (0 normal, 1 hora pico, 2 bajo tráfico), solicitud de cruce (0/1)
y milisegundos a simular. Agrega `csv` al final para obtener la salida en formato CSV.
