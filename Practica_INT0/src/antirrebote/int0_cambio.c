// ============================================================
//  int0_cambio.c - INT0 con antirrebote por software: modo "Cambio de estado"
//  ATmega328P @ 8 MHz - avr-gcc
//
//  Modo de activacion: ISC01=0, ISC00=1  (registro EICRA)
//  El LED cambia una vez al presionar y una vez al soltar.
//
//  Antirrebote: al entrar a la ISR se espera ANTIRREBOTE_MS a que el
//  contacto deje de rebotar, se vuelve a leer el pin y solo se invierte
//  el LED si la lectura confirma el evento. Al final se borra INTF0
//  para descartar los flancos que genero el rebote durante la espera.
//
//  Conexiones:
//    INT0 = PD2 (pin 4): switch del DIP o boton a GND (pull-up interno)
//    LED  = PB5 (pin 19): LED + resistencia de 220 ohm a GND
// ============================================================

#include <avr/io.h>           // Registros de I/O
#include <avr/interrupt.h>    // ISR(), sei()
#include <util/delay.h>       // Retardo del antirrebote

#define ANTIRREBOTE_MS 20     // Tiempo de espera para que pase el rebote

static volatile uint8_t ultimo;          // Ultimo estado estable del pin

// ----- Rutina de servicio de la interrupcion INT0 -----
ISR(INT0_vect)
{
    _delay_ms(ANTIRREBOTE_MS);           // Esperar a que pase el rebote

    uint8_t actual = PIND & (1 << PIND2);  // Leer el pin ya estable
    if (actual != ultimo) {              // Cambio real respecto al anterior?
        ultimo = actual;                 // Guardar el nuevo estado
        PORTB ^= (1 << PORTB5);          // Invertir LED
    }

    EIFR = (1 << INTF0);                 // Descartar flancos del rebote
}

int main(void)
{
    // ----- CONFIGURACION -----
    DDRB  |=  (1 << DDB5);                // PB5 (LED) como salida
    DDRD  &= ~(1 << DDD2);                // PD2 (INT0) como entrada
    PORTD |=  (1 << PORTD2);              // Pull-up interno en PD2

    _delay_ms(1);                         // Dejar que el pull-up suba el pin
    ultimo = PIND & (1 << PIND2);         // Estado inicial del pin

    EICRA = (1 << ISC00);                 // ISC01=0, ISC00=1
    EIFR   = (1 << INTF0);                // Limpiar bandera pendiente de INT0
    EIMSK  = (1 << INT0);                 // Habilitar INT0

    sei();                                // Habilitar interrupciones globales

    while (1) {
        // El programa principal no hace nada:
        // todo el trabajo lo hace la ISR
    }

    return 0;
}
