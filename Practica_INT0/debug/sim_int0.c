/*
 * sim_int0.c - Banco de prueba de INT0 (con y sin rebote) en simavr
 * Simula dos pulsaciones del boton en PD2 (0 = presionado). Cada vez que el
 * boton se presiona o se suelta, el contacto rebota: el pin cambia 6 veces
 * cada 0.5 ms antes de quedarse estable. Registra PB5 (LED) en CSV.
 * Compilar: gcc debug/sim_int0.c -o debug/sim_int0 -lsimavr -lelf
 * Usar:     ./debug/sim_int0 <archivo.elf> [0 = sin rebote]
 *
 * Nota: simavr 1.6 no implementa el borrado de INTF0 al escribir un 1 en EIFR
 * (en el chip real, escribir 1 borra la bandera). Este banco lo corrige con
 * escribir_eifr() para que la simulacion se comporte como la hoja de datos.
 */
#include <stdio.h>
#include <stdlib.h>
#include <simavr/sim_avr.h>
#include <simavr/sim_elf.h>
#include <simavr/avr_ioport.h>
#define F_CPU 8000000UL
#define INT0_vect_num 1         /* INT0 es el vector 1 (direccion 0x0002) */
#define MS(x) ((avr_cycle_count_t)((x) * (F_CPU / 1000)))
static avr_t *avr;
#define EIFR_ADDR 0x3C            /* EIFR: I/O 0x1C = direccion de datos 0x3C */
static void escribir_eifr(struct avr_t *a, avr_io_addr_t addr, uint8_t v, void *p)
{
    if (v & 0x01)                         /* escribir 1 en INTF0 la borra */
        for (int i = 0; i < a->interrupts.vector_count; i++)
            if (a->interrupts.vector[i]->vector == INT0_vect_num)
                avr_clear_interrupt(a, a->interrupts.vector[i]);
    a->data[addr] = 0;
}
static int led = -1;
static void cambio_pb5(avr_irq_t *irq, uint32_t v, void *p)
{
    if ((int)v == led) return;
    led = v;
    printf("%.3f,LED,%u\n", avr->cycle / (double)(F_CPU / 1000), v);
}
int main(int argc, char **argv)
{
    elf_firmware_t f = {0};
    if (argc < 2 || elf_read_firmware(argv[1], &f)) return 1;
    int rebote = (argc > 2) ? atoi(argv[2]) : 1;
    avr = avr_make_mcu_by_name("atmega328p");
    avr_init(avr); avr_load_firmware(avr, &f); avr->frequency = F_CPU;
    avr_irq_register_notify(avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('B'), 5), cambio_pb5, NULL);
    avr_register_io_write(avr, EIFR_ADDR, escribir_eifr, NULL);
    avr_irq_t *int0 = avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('D'), 2);
    /* Lista de eventos (ms, nivel) */
    double t[64]; int nv[64], n = 0;
    double base[] = {100, 400, 600, 900}; int fin[] = {0, 1, 0, 1};
    t[n] = 0; nv[n++] = 1;
    for (int i = 0; i < 4; i++) {
        double x = base[i];
        if (rebote)
            for (int k = 0; k < 6; k++) { t[n] = x; nv[n++] = (k % 2 == 0) ? fin[i] : !fin[i]; x += 0.5; }
        t[n] = x; nv[n++] = fin[i];
    }
    int k = 0;
    while (avr->cycle < MS(1100)) {
        if (k < n && avr->cycle >= MS(t[k])) {
            avr_raise_irq(int0, nv[k]);
            printf("%.3f,INT0,%d\n", avr->cycle / (double)(F_CPU / 1000), nv[k]);
            k++;
        }
        int st = avr_run(avr);
        if (st == cpu_Done || st == cpu_Crashed) break;
    }
    return 0;
}
