; ============================================================
;  main.S - Semaforo de crucero con paso peatonal (FSM)
;  ATmega328P @ 16 MHz - Sintaxis GNU Assembler (avr-gcc)
;
;  Las tablas de estados y de tiempos viven en la memoria
;  Flash. Para cambiar el comportamiento del semaforo solo
;  se modifican TABLA_ESTADOS y TABLA_TIEMPOS.
; ============================================================

#include <avr/io.h>

; ----- LEDs (salidas en PORTD) -----
.equ A_VERDE, 2
.equ A_ROJO,  3
.equ B_VERDE, 4
.equ B_ROJO,  5
.equ P_VERDE, 6
.equ P_ROJO,  7
.equ MASCARA_LEDS, 0xFC         ; PD2..PD7 como salida

; ----- DIP switch (entradas en PORTB, pull-up) -----
; Switch en ON = pin a GND = se lee 0
.equ DIP_PEATON, 0              ; solicitud de cruce
.equ DIP_MODO0,  1              ; bit 0 del modo
.equ DIP_MODO1,  2              ; bit 1 del modo

.equ BYTES_ESTADO, 4            ; bytes por renglon de TABLA_ESTADOS
.equ MEDIOS_CICLOS, 4           ; parpadeo: on-off-on-off (T/4 cada uno)

; ----- Uso de registros -----
; R16       temporal
; R17       estado actual
; R18       salidas del estado
; R19       mascara de parpadeo
; R20       contador de medios ciclos de parpadeo
; R21       siguiente estado sin solicitud de cruce
; R22       siguiente estado con solicitud de cruce
; R25:R24   tiempo en ms (parametro de DELAY_MS)

; ============================================================
;  Seccion de codigo
; ============================================================
.section .text
.global main

main:
    ; ----- CONFIGURACION -----
    ; LEDs como salida, todos apagados
    ldi R16, MASCARA_LEDS
    out _SFR_IO_ADDR(DDRD), R16
    clr R16
    out _SFR_IO_ADDR(PORTD), R16

    ; DIP switch como entrada con pull-up
    cbi _SFR_IO_ADDR(DDRB), DIP_PEATON
    cbi _SFR_IO_ADDR(DDRB), DIP_MODO0
    cbi _SFR_IO_ADDR(DDRB), DIP_MODO1
    sbi _SFR_IO_ADDR(PORTB), DIP_PEATON
    sbi _SFR_IO_ADDR(PORTB), DIP_MODO0
    sbi _SFR_IO_ADDR(PORTB), DIP_MODO1

    ; Estado inicial S0
    clr R17

MAIN_LOOP:
    ; ----- Z = TABLA_ESTADOS + estado * 4 -----
    ldi ZL, lo8(TABLA_ESTADOS)
    ldi ZH, hi8(TABLA_ESTADOS)
    mov R16, R17
    lsl R16
    lsl R16
    add ZL, R16
    clr R16
    adc ZH, R16

    ; ----- Leer renglon del estado desde Flash -----
    lpm R18, Z+                 ; salidas
    lpm R19, Z+                 ; mascara de parpadeo
    lpm R21, Z+                 ; siguiente sin solicitud
    lpm R22, Z                  ; siguiente con solicitud

    ; ----- Mostrar salidas -----
    out _SFR_IO_ADDR(PORTD), R18

    ; ----- Tiempo segun el modo del DIP -----
    rcall LEER_TIEMPO           ; R25:R24 = ms

    tst R19
    breq SIN_PARPADEO

    ; ----- Estado de atencion: parpadeo -----
    ; Medio ciclo = T / 4, asi el parpadeo completo dura T
    lsr R25
    ror R24
    lsr R25
    ror R24
    ldi R20, MEDIOS_CICLOS
PARPADEO:
    rcall DELAY_MS
    dec R20
    breq SIGUIENTE              ; 4 medios ciclos cumplidos
    in R16, _SFR_IO_ADDR(PORTD)
    eor R16, R19                ; invierte solo el LED que parpadea
    out _SFR_IO_ADDR(PORTD), R16
    rjmp PARPADEO

SIN_PARPADEO:
    rcall DELAY_MS

SIGUIENTE:
    ; ----- Elegir siguiente estado -----
    ; Si el switch del peaton esta en ON (pin = 0) usa R22
    in R16, _SFR_IO_ADDR(PINB)
    sbrs R16, DIP_PEATON
    mov R21, R22
    mov R17, R21
    rjmp MAIN_LOOP

; ============================================================
;  Subrutina: LEER_TIEMPO
;  Lee el modo del DIP y regresa el tiempo de TABLA_TIEMPOS
;  Salida: R25:R24 = tiempo en ms
; ============================================================
LEER_TIEMPO:
    in R16, _SFR_IO_ADDR(PINB)
    com R16                     ; switch ON = 1
    lsr R16                     ; DIP_MODO0 -> bit 0
    andi R16, 0x03              ; modo 0..3
    lsl R16                     ; cada tiempo ocupa 2 bytes

    ldi ZL, lo8(TABLA_TIEMPOS)
    ldi ZH, hi8(TABLA_TIEMPOS)
    add ZL, R16
    clr R16
    adc ZH, R16

    lpm R24, Z+                 ; byte bajo
    lpm R25, Z                  ; byte alto
    ret

; ============================================================
;  Subrutina: DELAY_MS
;  Retardo de R25:R24 milisegundos (16 MHz)
;  Conserva R25:R24
; ============================================================
DELAY_MS:
    movw R28, R24               ; Y = contador de ms
DELAY_MS1:
    ldi R26, lo8(3999)          ; X = vueltas para ~1 ms
    ldi R27, hi8(3999)
DELAY_MS2:
    sbiw R26, 1                 ; 2 ciclos
    brne DELAY_MS2              ; 2 ciclos -> 4 ciclos por vuelta
    sbiw R28, 1
    brne DELAY_MS1
    ret

; ============================================================
;  Tablas en memoria Flash
; ============================================================

; ----- Tabla de estados -----
; Salidas (PORTD): bit7 P_ROJO, bit6 P_VERDE, bit5 B_ROJO,
;                  bit4 B_VERDE, bit3 A_ROJO, bit2 A_VERDE
;
;       salidas parpadeo sig_sin sig_con
TABLA_ESTADOS:
    .byte 0xA4,   0x00,    1,      1    ; S0: A verde,    B rojo,     P rojo
    .byte 0xA4,   0x04,    2,      2    ; S1: A atencion, B rojo,     P rojo
    .byte 0x98,   0x00,    3,      3    ; S2: A rojo,     B verde,    P rojo
    .byte 0x98,   0x10,    4,      4    ; S3: A rojo,     B atencion, P rojo
    .byte 0xA8,   0x00,    0,      5    ; S4: A rojo,     B rojo,     P rojo
    .byte 0x68,   0x00,    0,      0    ; S5: A rojo,     B rojo,     P verde

; ----- Tabla de tiempos (ms) -----
; Indice = modo del DIP (MODO1 MODO0)
    .balign 2
TABLA_TIEMPOS:
    .word 1000                  ; 00: Normal
    .word 500                   ; 01: Hora pico
    .word 2000                  ; 10: Bajo trafico
    .word 1000                  ; 11: (no usado) Normal
