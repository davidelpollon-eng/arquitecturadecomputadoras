// ============================================================
//  main.c - Semaforo de crucero con paso peatonal (FSM)
//  ATmega328P @ 16 MHz - avr-gcc
//
//  Las tablas de estados y de tiempos viven en la memoria
//  Flash (PROGMEM). Para cambiar el comportamiento del
//  semaforo solo se modifican TABLA_ESTADOS y TABLA_TIEMPOS.
// ============================================================

#include <avr/io.h>          // Registros de I/O
#include <avr/pgmspace.h>    // PROGMEM y lectura de Flash (lpm)
#include <util/delay.h>      // Retardos

// ----- LEDs (salidas en PORTD) -----
#define A_VERDE  PD2
#define A_ROJO   PD3
#define B_VERDE  PD4
#define B_ROJO   PD5
#define P_VERDE  PD6
#define P_ROJO   PD7
#define MASCARA_LEDS 0xFC          // PD2..PD7 como salida

// ----- DIP switch (entradas en PORTB, pull-up) -----
// Switch en ON = pin a GND = se lee 0
#define DIP_PEATON PB0             // solicitud de cruce
#define DIP_MODO0  PB1             // bit 0 del modo
#define DIP_MODO1  PB2             // bit 1 del modo

#define MEDIOS_CICLOS 4            // parpadeo: on-off-on-off (T/4 cada uno)

// ----- Renglon de la tabla de estados -----
typedef struct {
    uint8_t salidas;               // valor que se escribe en PORTD
    uint8_t parpadeo;              // mascara del LED que parpadea
    uint8_t sig_sin;               // siguiente estado sin solicitud
    uint8_t sig_con;               // siguiente estado con solicitud
} Estado;

// ============================================================
//  Tablas en memoria Flash
// ============================================================

// Salidas (PORTD): bit7 P_ROJO, bit6 P_VERDE, bit5 B_ROJO,
//                  bit4 B_VERDE, bit3 A_ROJO, bit2 A_VERDE
const Estado TABLA_ESTADOS[] PROGMEM = {
    //  salidas parpadeo sig_sin sig_con
    {   0xA4,   0x00,    1,      1 },   // S0: A verde,    B rojo,     P rojo
    {   0xA4,   0x04,    2,      2 },   // S1: A atencion, B rojo,     P rojo
    {   0x98,   0x00,    3,      3 },   // S2: A rojo,     B verde,    P rojo
    {   0x98,   0x10,    4,      4 },   // S3: A rojo,     B atencion, P rojo
    {   0xA8,   0x00,    0,      5 },   // S4: A rojo,     B rojo,     P rojo
    {   0x68,   0x00,    0,      0 },   // S5: A rojo,     B rojo,     P verde
};

// Tiempo por estado en ms. Indice = modo del DIP (MODO1 MODO0)
const uint16_t TABLA_TIEMPOS[] PROGMEM = {
    1000,                          // 00: Normal
    500,                           // 01: Hora pico
    2000,                          // 10: Bajo trafico
    1000,                          // 11: (no usado) Normal
};

// ============================================================
//  Funciones auxiliares
// ============================================================

// Lee el modo del DIP y regresa el tiempo de TABLA_TIEMPOS
static uint16_t leer_tiempo(void) {
    uint8_t modo = (~PINB >> DIP_MODO0) & 0x03;   // switch ON = 1
    return pgm_read_word(&TABLA_TIEMPOS[modo]);
}

// Retardo de 'ms' milisegundos
// (_delay_ms necesita una constante, por eso se repite 1 ms)
static void delay_ms(uint16_t ms) {
    while (ms--) {
        _delay_ms(1);
    }
}

// ============================================================
//  Programa principal
// ============================================================
int main(void) {
    // ----- CONFIGURACION -----
    // LEDs como salida, todos apagados
    DDRD  = MASCARA_LEDS;
    PORTD = 0x00;

    // DIP switch como entrada con pull-up
    DDRB  &= ~((1 << DIP_PEATON) | (1 << DIP_MODO0) | (1 << DIP_MODO1));
    PORTB |=  ((1 << DIP_PEATON) | (1 << DIP_MODO0) | (1 << DIP_MODO1));

    uint8_t estado = 0;            // Estado inicial S0

    while (1) {
        // ----- Leer renglon del estado desde Flash -----
        uint8_t salidas  = pgm_read_byte(&TABLA_ESTADOS[estado].salidas);
        uint8_t parpadeo = pgm_read_byte(&TABLA_ESTADOS[estado].parpadeo);
        uint8_t sig_sin  = pgm_read_byte(&TABLA_ESTADOS[estado].sig_sin);
        uint8_t sig_con  = pgm_read_byte(&TABLA_ESTADOS[estado].sig_con);

        // ----- Mostrar salidas -----
        PORTD = salidas;

        // ----- Tiempo segun el modo del DIP -----
        uint16_t tiempo = leer_tiempo();

        if (parpadeo) {
            // Estado de atencion: medio ciclo = T / 4,
            // asi el parpadeo completo dura T
            for (uint8_t i = 1; i <= MEDIOS_CICLOS; i++) {
                delay_ms(tiempo / 4);
                if (i < MEDIOS_CICLOS) {
                    PORTD ^= parpadeo;      // invierte solo el LED que parpadea
                }
            }
        } else {
            delay_ms(tiempo);
        }

        // ----- Elegir siguiente estado -----
        // Si el switch del peaton esta en ON (pin = 0) usa sig_con
        if (!(PINB & (1 << DIP_PEATON))) {
            estado = sig_con;
        } else {
            estado = sig_sin;
        }
    }

    return 0;
}
