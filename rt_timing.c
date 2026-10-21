/*************************************************/
// rt_timing.c
// Implementacion de las funciones para el manejo 
// del tiempo real mediante Clocks y Timers.
/*************************************************/

#include "rt_timing.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NSEC_PER_SEC 1000000000ULL

/* Funcion matematica para sumar microsegundos y normalizar nanosegundos */
void timespec_add_us(struct timespec *t, uint64_t d)
{
    d *= 1000;
    d += t->tv_nsec;
    while (d >= NSEC_PER_SEC) {
        d -= NSEC_PER_SEC;
        t->tv_sec += 1;
    }
    t->tv_nsec = d;
}

/* ------------------------------------------------------------------ */
/* SECCION: POSIX Clocks (Para las tareas 3 y 4)                      */
/* ------------------------------------------------------------------ */

struct periodic_task_clock* start_periodic_clock(uint64_t offs, int period)
{
    struct periodic_task_clock *p;
    p = malloc(sizeof(struct periodic_task_clock));
    if (p == NULL) {
        return NULL;
    }

    /* Obtiene la hora actual exacta del sistema */
    clock_gettime(CLOCK_REALTIME, &p->next_arrival);
    
    /* Suma el offset inicial a la hora actual */
    timespec_add_us(&p->next_arrival, offs);
    
    /* Guarda el periodo para el calculo de los siguientes ciclos */
    p->period = period;
    p->count = 0;
    return p;
}

void wait_next_activation_clock(struct periodic_task_clock *t)
{
    /* Suspende el hilo hasta que el reloj absoluto alcance next_arrival */
    clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &t->next_arrival, NULL);
    
    /* Calcula inmediatamente la hora del proximo ciclo para evitar desfases */
    timespec_add_us(&t->next_arrival, t->period);
}

/* ------------------------------------------------------------------ */
/* SECCION: POSIX Timers (Para las tareas 1 y 2)                      */
/* ------------------------------------------------------------------ */

struct periodic_task_timer* start_periodic_timer(uint64_t offs, int period, int sigrt_offset)
{
    struct periodic_task_timer *p;
    int signotif = SIGRTMIN + sigrt_offset;

    p = malloc(sizeof(struct periodic_task_timer));
    if (p == NULL) {
        return NULL;
    }

    /* Limpia las estructuras para evitar datos residuales en memoria */
    memset(&p->timer_spec, 0, sizeof(p->timer_spec));
    memset(&p->sigev, 0, sizeof(p->sigev));

    /* Configura el conjunto de senales exclusivo para este hilo */
    sigemptyset(&p->sigset);
    sigaddset(&p->sigset, signotif);

    /* Configura el evento para que notifique usando la senal calculada */
    p->sigev.sigev_notify = SIGEV_SIGNAL;
    p->sigev.sigev_signo = signotif;

    /* Crea el temporizador vinculado al reloj del sistema */
    if (timer_create(CLOCK_REALTIME, &p->sigev, &p->timer_id) < 0) {
        perror("Error en timer_create");
        exit(-1);
    }

    /* Convierte el offset (microsegundos) a segundos y nanosegundos */
    p->timer_spec.it_value.tv_sec = offs / 1000000;
    p->timer_spec.it_value.tv_nsec = (offs % 1000000) * 1000;
    
    /* Convierte el periodo a segundos y nanosegundos */
    p->timer_spec.it_interval.tv_sec = period / 1000000;
    p->timer_spec.it_interval.tv_nsec = (period % 1000000) * 1000;

    /* Arma e inicia el temporizador */
    if (timer_settime(p->timer_id, 0, &p->timer_spec, NULL) < 0) {
        perror("Error en timer_settime");
        exit(-1);
    }

    p->period = period;
    p->count = 0;
    return p;
}

void wait_next_activation_timer(struct periodic_task_timer *t)
{
    int sig;
    /* El hilo se suspende esperando exclusivamente su senal correspondiente */
    if (sigwait(&t->sigset, &sig) < 0) {
        perror("Error en sigwait");
        exit(-1);
    }
}