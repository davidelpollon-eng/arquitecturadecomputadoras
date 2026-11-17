/*
 * sim_semaforo.c - Banco de prueba del semaforo en simavr
 *
 * Carga main.elf en un ATmega328P simulado a F_CPU (8 MHz), fija el DIP switch
 * (PB0 = solicitud de cruce, PB1/PB2 = modo) y registra cada cambio de
 * PORTD con su tiempo en milisegundos.
 *
 * Compilar (desde la carpeta del proyecto):
 *            gcc debug/sim_semaforo.c -o debug/sim_semaforo -lsimavr -lelf
 * Usar:      ./sim_semaforo <archivo.elf> <modo 0-2> <peaton 0/1> <ms> [csv]
 *            modo: 0 = normal, 1 = hora pico, 2 = bajo trafico
 * Ejemplo:   ./debug/sim_semaforo release/main.elf 1 1 4000
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <simavr/sim_avr.h>
#include <simavr/sim_elf.h>
#include <simavr/avr_ioport.h>

#define F_CPU 8000000UL           /* igual que en el makefile */
#define CICLOS_MS (F_CPU / 1000)

static avr_t *avr;
static int ultimo = -1;
static int modo_csv = 0;

static void cambio_portd(avr_irq_t *irq, uint32_t v, void *param)
{
    v &= 0xFC;                          /* solo PD2..PD7 */
    if ((int)v == ultimo) return;
    ultimo = v;
    double ms = avr->cycle / (double)CICLOS_MS;
    if (modo_csv) { printf("%.3f,%u\n", ms, v); return; }
    printf("%9.1f ms  PORTD=0x%02X  A:%s%s  B:%s%s  P:%s%s\n", ms, v,
           v & 0x04 ? "V" : "-", v & 0x08 ? "R" : "-",
           v & 0x10 ? "V" : "-", v & 0x20 ? "R" : "-",
           v & 0x40 ? "V" : "-", v & 0x80 ? "R" : "-");
}

int main(int argc, char **argv)
{
    if (argc < 5) {
        fprintf(stderr, "uso: %s <archivo.elf> <modo 0-2> <peaton 0/1> <ms> [csv]\n", argv[0]);
        return 1;
    }
    int modo = atoi(argv[2]), peaton = atoi(argv[3]);
    double ms = atof(argv[4]);
    modo_csv = (argc > 5 && strcmp(argv[5], "csv") == 0);

    elf_firmware_t f = {0};
    if (elf_read_firmware(argv[1], &f)) { fprintf(stderr, "no se pudo leer %s\n", argv[1]); return 1; }
    avr = avr_make_mcu_by_name("atmega328p");
    avr_init(avr);
    avr_load_firmware(avr, &f);
    avr->frequency = F_CPU;

    avr_irq_register_notify(avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('D'), IOPORT_IRQ_PIN_ALL),
                            cambio_portd, NULL);
    avr_irq_t *pb = avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('B'), 0);

    /* El DIP se fija despues de que el programa activa los pull-ups.
       Switch en ON = pin a GND = 0. */
    int dip_listo = 0;
    while (avr->cycle < ms * CICLOS_MS) {
        if (!dip_listo && avr->cycle > 1000) {
            dip_listo = 1;
            avr_raise_irq(pb + 0, peaton ? 0 : 1);
            avr_raise_irq(pb + 1, (modo & 1) ? 0 : 1);
            avr_raise_irq(pb + 2, (modo & 2) ? 0 : 1);
        }
        int estado = avr_run(avr);
        if (estado == cpu_Done || estado == cpu_Crashed) break;
    }
    return 0;
}
