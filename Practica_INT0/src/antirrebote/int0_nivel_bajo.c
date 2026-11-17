// ============================================================
//  int0_nivel_bajo.c - INT0 con antirrebote por software: modo "Nivel bajo"
//  ATmega328P @ 8 MHz - avr-gcc
//
//  Modo de activacion: ISC01=0, ISC00=0  (registro EICRA)
//  Mientras el pin este en 0 la interrupcion se dispara una y otra vez:
//  el LED parpadea todo el tiempo que el switch/boton este presionado.
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

// ----- Rutina de servicio de la interrupcion INT0 -----
ISR(INT0_vect)
{
    _delay_ms(ANTIRREBOTE_MS);           // Esperar a que pase el rebote

    if (!(PIND & (1 << PIND2))) {        // Sigue presionado (pin en 0)?
        PORTB ^= (1 << PORTB5);          // Si: invertir LED
        _delay_ms(100 - ANTIRREBOTE_MS); // Completar 100 ms por cambio
    }
    // En nivel bajo no hay bandera que borrar: si el pin sigue en 0,
    // la interrupcion se vuelve a disparar al salir de la ISR.
}

int main(void)
{
    // ----- CONFIGURACION -----
    DDRB  |=  (1 << DDB5);                // PB5 (LED) como salida
    DDRD  &= ~(1 << DDD2);                // PD2 (INT0) como entrada
    PORTD |=  (1 << PORTD2);              // Pull-up interno en PD2

    EICRA = 0;                            // ISC01=0, ISC00=0
    EIFR   = (1 << INTF0);                // Limpiar bandera pendiente de INT0
    EIMSK  = (1 << INT0);                 // Habilitar INT0

    sei();                                // Habilitar interrupciones globales

    while (1) {
        // El programa principal no hace nada:
        // todo el trabajo lo hace la ISR
    }

    return 0;
}
