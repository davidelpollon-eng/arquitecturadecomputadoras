# Arquitectura de Computadoras

Repositorio de tareas, laboratorios y ejercicios de la materia **Arquitectura de Computadoras**.

**Alumno:** David Muñoz Barrón
**Carrera:** Ingeniería en Sistemas — Grupo 3C
**Universidad:** Universidad Autónoma de Aguascalientes

## Contenido

| Carpeta / Archivo | Descripción |
|---|---|
| [`Tarea_Fibonacci`](./Tarea_Fibonacci) | Generador de la serie de Fibonacci en C; análisis de las 4 etapas de compilación con GCC (preprocesamiento, compilación, ensamblado y enlazado), identificación de tipos de archivo con `file`, y análisis de 5 instrucciones del código ensamblador generado. |
| [`reporte_ciclo_instruccion.pdf`](./reporte_ciclo_instruccion.pdf) | Reporte de ejercicio hecho en clase: codificación a lenguaje máquina de un algoritmo aritmético para una arquitectura CPU imaginaria (5 opcodes, instrucción de 9 bits), y simulación etapa por etapa (Fetch, Decode, Execute, Memory/WB) de las 7 instrucciones resultantes. |
| [`ejemplo_01.S`](./ejemplo_01.S) / `ejemplo_01.elf` | Ejercicio en ensamblador AVR: bucle con instrucciones `ldi`, `mov` y `rjmp`, ensamblado y enlazado a un ejecutable ELF para microcontroladores AVR. |

> Esta tabla se irá actualizando conforme se agreguen nuevas tareas al repositorio.

## Archivos de prueba

Algunos archivos en la raíz del repositorio fueron pruebas iniciales para familiarizarse con Git y GitHub, no corresponden a tareas formales de la materia:

- **`hola_mundo.c`** — Prueba de "Hola Mundo" al subir código por primera vez desde terminal.
- **`operasiones.c`** — Ejercicio en clase de una suma simple.
- **`tarea_01/`** — Carpeta de prueba para verificar la subida de carpetas completas al repositorio (contiene archivos similares a los de `Tarea_Fibonacci`).

## Entorno de trabajo

- **Sistema:** Ubuntu (máquina virtual)
- **Compilador:** GCC (`gcc`) para ejercicios en C, `avr-gcc`/binutils para ensamblador AVR
- **Herramientas:** `gcc`, `file`, `nano`

## Estructura típica de cada tarea

Cada carpeta de tarea incluye, cuando aplica:
- Código fuente (`.c` o `.S`)
- Archivos intermedios generados por el compilador (`.i`, `.s`, `.o`)
- Ejecutable (o `.elf` en el caso de ensamblador AVR)
- Reporte con explicación, resultados y capturas de pantalla

## Cómo compilar los ejercicios en C

```bash
gcc -E archivo.c -o archivo.i   # Preprocesamiento
gcc -S archivo.i -o archivo.s   # Compilación a ensamblador
gcc -c archivo.s -o archivo.o   # Ensamblado a código objeto
gcc archivo.o -o ejecutable     # Enlazado
```
