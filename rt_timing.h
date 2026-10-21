/*************************************************/
// rt_timing.h
// Definiciones y estructuras para el manejo del 
// tiempo real mediante POSIX Clocks y POSIX Timers.
/*************************************************/

#ifndef RT_TIMING_H
#define RT_TIMING_H

#include <time.h>
#include <signal.h>
#include <stdint.h>

/* 
 * Estructura para tareas manejadas con POSIX Clocks (Tiempo Absoluto).
 * Utilizada para las tareas 3 y 4.
 */
struct periodic_task_clock {
    struct timespec next_arrival; /* Almacena la hora exacta de la proxima ejecucion */
    int period;                   /* Periodo de la tarea en microsegundos */
    int count;
};




/* 
 * Estructura para tareas manejadas con POSIX Timers (Señales).
 * Utilizada para las tareas 1 y 2.
 */
struct periodic_task_timer {
    sigset_t sigset;              /* Conjunto de señales que el hilo debe esperar */
    struct itimerspec timer_spec; /* Configuracion de los tiempos del temporizador */
    struct sigevent sigev;        /* Configuracion del evento que dispara la señal */
    timer_t timer_id;             /* Identificador del temporizador en el sistema */
    int period;                   /* Periodo de la tarea en microsegundos */
    int count; 
};

/* 
 * Funcion utilitaria para sumar microsegundos a una estructura timespec 
 * manteniendo el formato correcto (segundos y nanosegundos).
 */
void timespec_add_us(struct timespec *t, uint64_t d);

/* 
 * Prototipos para la inicializacion y suspension mediante POSIX Clocks 
 */
struct periodic_task_clock* start_periodic_clock(uint64_t offs, int period);
void wait_next_activation_clock(struct periodic_task_clock *t);

/* 
 * Prototipos para la inicializacion y suspension mediante POSIX Timers 
 */
struct periodic_task_timer* start_periodic_timer(uint64_t offs, int period, int sigrt_offset);
void wait_next_activation_timer(struct periodic_task_timer *t);

#endif /* RT_TIMING_H */