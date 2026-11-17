// ============================================================
//  timer1_3vel.c - Timer1 (CTC) + INT0/INT1: 3 velocidades con el DIP
//  ATmega328P @ 8 MHz (oscilador interno) - avr-gcc
//
//  - El Timer1 genera una interrupcion cada 1 ms.
//  - Los switches 1 y 2 del DIP eligen cada cuanto cambia el LED:
//
//        Switch 1   Switch 2   Velocidad
//        OFF        OFF        Lenta   (1000 ms)
//        ON         OFF        Media   ( 500 ms)
//        cualquier  ON         Rapida  ( 100 ms)
//
//  - INT0 (switch 1) e INT1 (switch 2) estan en modo "cualquier
//    cambio": al mover un switch, su ISR vuelve a leer los dos.
//
//  Conexiones:
//    LED       = PB5 (pin 19) con 220 ohm a GND
//    Switch 1  = PD2 (pin 4, INT0) a GND (pull-up interno)
//    Switch 2  = PD3 (pin 5, INT1) a GND (pull-up interno)
// ============================================================

#include <avr/io.h>           // Registros
#include <avr/interrupt.h>    // ISR() y sei()
#include <stdint.h>           // uint16_t

#define TOPE_1MS    124       // OCR1A para 1 ms con 8 MHz y prescaler 64

#define MS_LENTO    1000      // Los dos switches en OFF
#define MS_MEDIO    500       // Switch 1 en ON
#define MS_RAPIDO   100       // Switch 2 en ON

volatile uint16_t contador_ms = 0;          // Milisegundos transcurridos
volatile uint16_t intervalo   = MS_LENTO;   // Cada cuantos ms cambia el LED

// Lee los dos switches y elige la velocidad (switch ON = pin en 0)
static void leer_switches(void)
{
    if (!(PIND & (1 << PIND3))) {        // Switch 2 en ON
        intervalo = MS_RAPIDO;
    } else if (!(PIND & (1 << PIND2))) { // Switch 1 en ON
        intervalo = MS_MEDIO;
    } else {                             // Los dos en OFF
        intervalo = MS_LENTO;
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

// ----- INT0: se movio el switch 1 -----
ISR(INT0_vect)
{
    leer_switches();
}

// ----- INT1: se movio el switch 2 -----
ISR(INT1_vect)
{
    leer_switches();
}

int main(void)
{
    // ----- LED -----
    DDRB |= (1 << DDB5);                         // PB5 (pin 19) como salida

    // ----- Switches 1 y 2 -----
    DDRD  &= ~((1 << DDD2) | (1 << DDD3));       // PD2 y PD3 como entrada
    PORTD |=  (1 << PORTD2) | (1 << PORTD3);     // Pull-up: OFF = 1, ON = 0

    // ----- Timer1 en modo CTC, 1 ms -----
    TCCR1A = 0;                                  // Sin salidas PWM
    TCCR1B = 0;                                  // Timer detenido
    TCNT1  = 0;                                  // Contador en 0
    OCR1A  = TOPE_1MS;                           // 1 ms
    TCCR1B |= (1 << WGM12);                      // Modo CTC
    TCCR1B |= (1 << CS11) | (1 << CS10);         // Prescaler 64 y arranca
    TIMSK1 |= (1 << OCIE1A);                     // Interrupcion por comparacion A

    // ----- INT0 e INT1 en modo cualquier cambio -----
    EICRA = (1 << ISC00) | (1 << ISC10);         // Cualquier cambio en pin 4 y pin 5
    EIFR  = (1 << INTF0) | (1 << INTF1);         // Borrar banderas pendientes
    EIMSK = (1 << INT0)  | (1 << INT1);          // Habilitar INT0 e INT1

    leer_switches();                             // Velocidad segun los switches al arrancar

    sei();                                       // Interrupciones globales

    while (1) {
        // No hace nada: el Timer1, INT0 e INT1 hacen todo
    }

    return 0;
}
