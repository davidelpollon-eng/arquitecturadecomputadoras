// ============================================================
//  timer1_ctc.c - Parpadeo de un LED con el Timer1 en modo CTC
//  ATmega328P @ 8 MHz (oscilador interno) - avr-gcc
//
//  El Timer1 genera una interrupcion cada 1 ms. La ISR cuenta
//  milisegundos y cada 500 ms invierte el LED: el LED se prende
//  500 ms y se apaga 500 ms (1 Hz).
//
//  Calculo (F_CPU = 8 MHz, prescaler = 64):
//    reloj del timer = 8 000 000 / 64 = 125 000 Hz -> 8 us por cuenta
//    1 ms / 8 us = 125 cuentas -> OCR1A = 125 - 1 = 124
//
//  (En el pizarron: F_CPU = 16 MHz -> 16 000 000 / 64 = 250 000 Hz
//   -> 250 cuentas -> OCR1A = 249)
//
//  Conexion: LED en PB5 (pin 19) con resistencia de 220 ohm a GND
// ============================================================

#include <avr/io.h>           // Registros: DDRB, TCCR1A, OCR1A...
#include <avr/interrupt.h>    // ISR() y sei()
#include <stdint.h>           // uint16_t

#define MS_PARPADEO 500       // Cada cuantos ms cambia el LED
#define TOPE_1MS    124       // OCR1A para 1 ms con 8 MHz y prescaler 64

volatile uint16_t contador_ms = 0;   // Milisegundos transcurridos

// ----- Se ejecuta cada 1 ms (cuando TCNT1 llega a OCR1A) -----
ISR(TIMER1_COMPA_vect)
{
    contador_ms++;                       // Paso 1 ms mas

    if (contador_ms >= MS_PARPADEO) {    // Ya pasaron 500 ms?
        PORTB ^= (1 << PB5);             // Invertir LED
        contador_ms = 0;                 // Empezar a contar otra vez
    }
}

int main(void)
{
    DDRB |= (1 << DDB5);                 // PB5 (LED) como salida

    // ----- Configuracion del Timer1 -----
    TCCR1A = 0;                          // Sin salidas PWM, modo normal
    TCCR1B = 0;                          // Timer detenido mientras se configura
    TCNT1  = 0;                          // Contador en 0

    OCR1A  = TOPE_1MS;                   // Valor de comparacion: 1 ms

    TCCR1B |= (1 << WGM12);              // Modo CTC: reinicia al llegar a OCR1A
    TCCR1B |= (1 << CS11) | (1 << CS10); // Prescaler 64: arranca el timer

    TIMSK1 |= (1 << OCIE1A);             // Habilitar interrupcion por comparacion A

    sei();                               // Habilitar interrupciones globales

    while (1) {
        // No hace nada: el timer y la ISR hacen todo
    }

    return 0;
}
