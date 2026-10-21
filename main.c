/*************************************************/
// main.c
// Punto de entrada del sistema de control del automovil.
// Inicializacion de senales, hilos y gestion del ciclo de vida.
/*************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>

#include "rt_timing.h"
#include "car_tasks.h"

#define SECONDS_TO_RUN 10

int main(int argc, char *argv[])
{
    pthread_t thread_1_id, thread_2_id, thread_3_id, thread_4_id;
    sigset_t alarm_sigset;
    int i, err;

    printf("\n==================================================\n");
    printf("   SISTEMA DE CONTROL DE AUTOMOVIL EN TIEMPO REAL   \n");
    printf("==================================================\n\n");

    /* 
     * PASO 1: Bloquear senales de tiempo real en el hilo principal.
     * Es fundamental hacerlo antes de crear los hilos para que todos
     * los hilos hijos hereden esta mascara y puedan usar sigwait.
     */
    sigemptyset(&alarm_sigset);
    for (i = SIGRTMIN; i <= SIGRTMAX; i++) {
        sigaddset(&alarm_sigset, i);
    }
    sigprocmask(SIG_BLOCK, &alarm_sigset, NULL);

    /* 
     * PASO 2: Configurar las estructuras de tiempo para cada tarea
     * segun los periodos requeridos en la Tabla 1.
     */

    /* Tarea 1: Control de Estabilidad (20 ms = 20000 us) - POSIX Timers (Signal +1) */
    struct periodic_task_timer *t1 = start_periodic_timer(100000, 20000, 1);

    /* Tarea 2: Control de Traccion (40 ms = 40000 us) - POSIX Timers (Signal +2) */
    struct periodic_task_timer *t2 = start_periodic_timer(150000, 40000, 2);

    /* Tarea 3: Inyeccion de Combustible (80 ms = 80000 us) - POSIX Clocks */
    struct periodic_task_clock *t3 = start_periodic_clock(200000, 80000);

    /* Tarea 4: Monitoreo de Temperatura (160 ms = 160000 us) - POSIX Clocks */
    struct periodic_task_clock *t4 = start_periodic_clock(250000, 160000);

    /* 
     * PASO 3: Creacion y activacion de los cuatro hilos concurrentes.
     */
    err = pthread_create(&thread_1_id, NULL, tarea_control_estabilidad, (void*) t1);
    if (err != 0) {
        fprintf(stderr, "Error al crear el hilo de Control de Estabilidad\n");
        return -1;
    }

    err = pthread_create(&thread_2_id, NULL, tarea_control_traccion, (void*) t2);
    if (err != 0) {
        fprintf(stderr, "Error al crear el hilo de Control de Traccion\n");
        return -1;
    }

    err = pthread_create(&thread_3_id, NULL, tarea_inyeccion_combustible, (void*) t3);
    if (err != 0) {
        fprintf(stderr, "Error al crear el hilo de Inyeccion de Combustible\n");
        return -1;
    }

    err = pthread_create(&thread_4_id, NULL, tarea_monitoreo_temperatura, (void*) t4);
    if (err != 0) {
        fprintf(stderr, "Error al crear el hilo de Monitoreo de Temperatura\n");
        return -1;
    }

    printf("Sistema iniciado correctamente. Ejecutando durante %d segundos...\n\n", SECONDS_TO_RUN);

    /* 
     * PASO 4: El hilo principal se suspende mientras las tareas trabajan en segundo plano.
     */
    sleep(SECONDS_TO_RUN);

     /* 
     * PASO 5: Imprimir resumen deterministico
     */
    imprimir_resumen(t1, t2, t3, t4, SECONDS_TO_RUN);

    printf("\nTiempo de ejecucion finalizado. Cerrando sistema...\n");

    /* 
     * PASO 6: Limpieza de recursos del sistema.
     */

    destruir_sincronizacion();

    free(t1);
    free(t2);
    free(t3);
    free(t4);

    return 0;
}