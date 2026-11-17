# Práctica Timer1 — parpadeo con interrupción por comparación (CTC)

El Timer1 genera una interrupción cada 1 ms y la ISR invierte el LED cada 500 ms (1 Hz).

- Chip a **8 MHz** (oscilador interno): `OCR1A = 124` con prescaler 64.
- En el pizarrón está para **16 MHz**: `OCR1A = 249`.

## Circuito
El mismo de la práctica INT0: LED en **PB5 (pin 19)** con 220 Ω a GND. El botón y el DIP no se usan.

## Grabar
```bash
sudo make flash
```

## Versión combinada: Timer1 + INT0 (el switch elige la velocidad)
`src/timer1_switch.c`: el Timer1 cuenta milisegundos y el switch 1 del DIP (pin 4, INT0
en modo "cualquier cambio") elige el intervalo: OFF = 500 ms, ON = 100 ms.

```bash
sudo make flash SRC=src/timer1_switch.c
```
Circuito: el de la práctica Timer1 más el switch 1 del DIP entre el pin 4 y GND.

## Versión de 3 velocidades (la que graba `sudo make flash`)
`src/timer1_3vel.c`: switch 1 → pin 4 (INT0), switch 2 → pin 5 (INT1), ambos a GND.

| Switch 1 | Switch 2 | Velocidad |
|---|---|---|
| OFF | OFF | Lenta (1000 ms) |
| ON | OFF | Media (500 ms) |
| cualquiera | ON | Rápida (100 ms) |

```bash
sudo make flash                                  # 3 velocidades (por defecto)
sudo make flash SRC=src/timer1_ctc.c             # el del pizarrón
sudo make flash SRC=src/timer1_switch.c          # 2 velocidades
```
