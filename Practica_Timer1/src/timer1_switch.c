// ============================================================
//  timer1_switch.c - Timer1 (CTC) + INT0: el switch elige la velocidad
//  ATmega328P @ 8 MHz (oscilador interno) - avr-gcc
//
//  - El Timer1 genera una interrupcion cada 1 ms (igual que timer1_ctc.c).
//  - El switch 1 del DIP (pin 4, INT0) elige cada cuanto cambia el LED:
//        switch OFF (pin en 1) -> 500 ms  (parpadeo lento)
//        switch ON  (pin en 0) -> 100 ms  (parpadeo rapido)
//  - INT0 esta en modo "cualquier cambio": cada vez que mueves el
//    switch, la ISR de INT0 lee el pin y actualiza la velocidad.
//
//  Conexiones:
//    LED  = PB5 (pin 19) con 220 ohm a GND
//    DIP  = switch 1 entre PD2 (pin 4) y GND (pull-up interno)
// ============================================================

#include <avr/io.h>           // Registros
#include <avr/interrupt.h>    // ISR() y sei()
#include <stdint.h>           // uint16_t

#define TOPE_1MS    124       // OCR1A para 1 ms con 8 MHz y prescaler 64
#define MS_LENTO    500       // Switch OFF
#define MS_RAPIDO   100       // Switch ON

volatile uint16_t contador_ms = 0;          // Milisegundos transcurridos
volatile uint16_t intervalo   = MS_LENTO;   // Cada cuantos ms cambia el LED

// Lee el switch y elige la velocidad
static void leer_switch(void)
{
    if (PIND & (1 << PIND2)) {   // Pin en 1: switch OFF
        intervalo = MS_LENTO;
    } else {                     // Pin en 0: switch ON
        intervalo = MS_RAPIDO;
    }
}

// ----- Timer1: se ejecuta cada 1 ms -----
ISR(TIMER1_COMPA_vect)
{
    contador_ms++;                       // Paso 1 ms
    if (contador_ms >= intervalo) {      // Ya paso el intervalo?
        PORTB ^= (1 << PB5);             // Invertir LED
        contador_ms = 0;                 // Contar de nuevo
    }
}

// ----- INT0: se ejecuta cada vez que se mueve el switch -----
ISR(INT0_vect)
{
    leer_switch();                       // Actualizar la velocidad
}

int main(void)
{
    // ----- LED -----
    DDRB |= (1 << DDB5);                 // PB5 (pin 19) como salida

    // ----- Switch en INT0 -----
    DDRD  &= ~(1 << DDD2);               // PD2 (pin 4) como entrada
    PORTD |=  (1 << PORTD2);             // Pull-up: OFF = 1, ON = 0

    // ----- Timer1 en modo CTC, 1 ms -----
    TCCR1A = 0;                          // Sin salidas PWM
    TCCR1B = 0;                          // Timer detenido
    TCNT1  = 0;                          // Contador en 0
    OCR1A  = TOPE_1MS;                   // 1 ms
    TCCR1B |= (1 << WGM12);              // Modo CTC
    TCCR1B |= (1 << CS11) | (1 << CS10); // Prescaler 64 y arranca
    TIMSK1 |= (1 << OCIE1A);             // Interrupcion por comparacion A

    // ----- INT0 en modo cualquier cambio -----
    EICRA = (1 << ISC00);                // Cualquier cambio en el pin 4
    EIFR  = (1 << INTF0);                // Borrar bandera pendiente
    EIMSK = (1 << INT0);                 // Habilitar INT0

    leer_switch();                       // Velocidad segun como este el switch al arrancar

    sei();                               // Interrupciones globales

    while (1) {
        // No hace nada: el Timer1 y la INT0 hacen todo
    }

    return 0;
}
