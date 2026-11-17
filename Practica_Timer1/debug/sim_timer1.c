/*
 * sim_timer1.c - Banco de prueba de timer1_3vel.c en simavr (switches en PD2 y PD3)
 * Mueve el switch 1 (PD2) y el switch 2 (PD3) durante 6 s para recorrer las
 * 3 velocidades y registra cada cambio de PB5 (LED) en CSV.
 * Con el segundo argumento en 1, cada cambio de PD2 lleva rebote simulado.
 * Compilar: gcc debug/sim_timer1.c -o debug/sim_timer1 -lsimavr -lelf
 * Usar:     ./debug/sim_timer1 <archivo.elf> 0
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
    if (v & 0x03)                         /* escribir 1 en INTF0 la borra */
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
    avr_irq_t *int1 = avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('D'), 3);
    avr_raise_irq(int1, 1);
    int k2 = 0; double t2[] = {3500, 4500}; int v2[] = {0, 1};
    /* Lista de eventos (ms, nivel) */
    double t[64]; int nv[64], n = 0;
    double base[] = {2500, 4000, 4800, 5600}; int fin[] = {0, 1, 0, 1};
    t[n] = 0; nv[n++] = 1;
    for (int i = 0; i < 4; i++) {
        double x = base[i];
        if (rebote)
            for (int k = 0; k < 6; k++) { t[n] = x; nv[n++] = (k % 2 == 0) ? fin[i] : !fin[i]; x += 0.5; }
        t[n] = x; nv[n++] = fin[i];
    }
    int k = 0;
    while (avr->cycle < MS(6000)) {
        if (k < n && avr->cycle >= MS(t[k])) {
            avr_raise_irq(int0, nv[k]);
            printf("%.3f,INT0,%d\n", avr->cycle / (double)(F_CPU / 1000), nv[k]);
            k++;
        }
        if (k2 < 2 && avr->cycle >= MS(t2[k2])) { avr_raise_irq(int1, v2[k2]); printf("%.3f,INT1,%d\n", avr->cycle/8000.0, v2[k2]); k2++; }
        int st = avr_run(avr);
        if (st == cpu_Done || st == cpu_Crashed) break;
    }
    return 0;
}
