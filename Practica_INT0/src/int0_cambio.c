// ============================================================
//  int0_cambio.c - Interrupcion externa INT0: modo "Cualquier cambio"
//  ATmega328P @ 8 MHz - avr-gcc
//
//  Modo de activacion: ISC01 = 0, ISC00 = 1  (registro EICRA)
//  La interrupcion se dispara al presionar Y al soltar: el LED cambia
//  en cada transicion del pin INT0.
//
//  Conexiones:
//    INT0 = PD2 (pin 4): switch del DIP o boton a GND (pull-up interno)
//    LED  = PB5 (pin 19): LED + resistencia de 220 ohm a GND
// ============================================================

#include <avr/io.h>           // Registros de I/O
#include <avr/interrupt.h>    // ISR(), sei()

// ----- Rutina de servicio de la interrupcion INT0 -----
ISR(INT0_vect)
{
    PORTB ^= (1 << PORTB5);              // Invertir LED
}

int main(void)
{
    // ----- CONFIGURACION -----
    DDRB  |=  (1 << DDB5);                // PB5 (LED) como salida
    DDRD  &= ~(1 << DDD2);                // PD2 (INT0) como entrada
    PORTD |=  (1 << PORTD2);              // Pull-up interno en PD2

    EICRA = (1 << ISC00);                // ISC01=0, ISC00=1
    EIFR   = (1 << INTF0);                // Limpiar bandera pendiente de INT0
    EIMSK  = (1 << INT0);                 // Habilitar INT0

    sei();                                // Habilitar interrupciones globales

    while (1) {
        // El programa principal no hace nada:
        // todo el trabajo lo hace la ISR
    }

    return 0;
}
