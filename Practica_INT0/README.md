# Práctica: modos de activación de la interrupción externa INT0 (ATmega328P)

Cuatro programas en C, uno por cada modo de `EICRA` (bits `ISC01` y `ISC00`):

| Programa | ISC01 ISC00 | Modo | Qué hace el LED |
|---|---|---|---|
| `src/int0_nivel_bajo.c` | 0 0 | Nivel bajo | Parpadea todo el tiempo que el pin está en 0 |
| `src/int0_cambio.c` | 0 1 | Cualquier cambio | Cambia al presionar **y** al soltar |
| `src/int0_flanco_bajada.c` | 1 0 | Flanco de bajada | Cambia solo al **presionar** |
| `src/int0_flanco_subida.c` | 1 1 | Flanco de subida | Cambia solo al **soltar** |

## Conexiones

- **INT0 = PD2 (pin 4):** el switch del DIP o el botón pulsador, del pin a GND (pull-up interno).
- **LED = PB5 (pin 19):** LED + resistencia de 220 Ω a GND.

## Compilar y grabar

La regla `flash` vuelve a compilar, así que el archivo se indica en la misma orden:

```bash
sudo make flash SRC=src/int0_flanco_bajada.c
```

## Versión con antirrebote por software

En `src/antirrebote/` están los mismos 4 programas con antirrebote: la ISR espera
20 ms (`ANTIRREBOTE_MS`), vuelve a leer el pin, solo invierte el LED si el evento
se confirma y al final borra `INTF0` para descartar los flancos del rebote.

```bash
sudo make flash SRC=src/antirrebote/int0_flanco_bajada.c
```

## Documentación y simulación

- `docs/Practica_INT0_ATmega328P_IEEE.pdf` (y `.docx`): reporte en formato IEEE.
- `debug/sim_int0.c`: banco de prueba en simavr. Simula dos pulsaciones en PD2,
  con rebote (por defecto) o sin rebote (`0`), y registra PB5.

```bash
gcc debug/sim_int0.c -o debug/sim_int0 -lsimavr -lelf
make SRC=src/antirrebote/int0_cambio.c
./debug/sim_int0 release/main.elf        # con rebote
./debug/sim_int0 release/main.elf 0      # sin rebote
```

## Volver al semáforo

1. Desconecta el USB.
2. Quita de la columna del pin 4 el cable al switch y el botón, y regresa ahí el cable del **LED A verde**.
3. Regresa el cable del **switch 1** del DIP a la columna del **pin 14**.
4. Conecta el USB y graba el semáforo:

```bash
cd ~/codigos/Examen_P02_Semaforo
sudo make flash
```
