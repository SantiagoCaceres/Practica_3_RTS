/*************************************************/
// car_tasks.c
// Implementacion del cuerpo de las tareas (Job Bodies)
// y gestion de la sincronizacion del recurso compartido.
/*************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "car_tasks.h"
#include "rt_timing.h"

/* 
 * Recurso compartido e inicializacion de elementos de sincronizacion
 */
struct EstadoVehiculo estado_carro = {
    .velocidad_rueda_izq = 80.0f,
    .velocidad_rueda_der = 80.0f,
    .presion_acelerador = 25.0f,
    .inyeccion_calculada = 10.0f,
    .temperatura_motor = 90.0f,
    .datos_nuevos = 0
};

/* Mutex para exclusion mutua sobre el recurso compartido */
pthread_mutex_t mutex_estado = PTHREAD_MUTEX_INITIALIZER;

/* Variable condicional para notificar cuando hay datos de sensores actualizados */
pthread_cond_t cond_datos_nuevos = PTHREAD_COND_INITIALIZER;


/* ------------------------------------------------------------------ */
/* Tarea 1: Control de Estabilidad (ESC/ESP) - 20 ms - POSIX Timers   */
/* ------------------------------------------------------------------ */
void* tarea_control_estabilidad(void* arg)
{
    struct periodic_task_timer* t = (struct periodic_task_timer*) arg;
    static int iteracion = 0;
    
    while (1) {
        /* Espera la llegada de la senal del temporizador */
        wait_next_activation_timer(t);

        /* Seccion Critica: Modifica velocidades de ruedas para simular correccion */
        pthread_mutex_lock(&mutex_estado);
        t->count++;
        iteracion++;
        /* Simula ajuste dinamico de diferencia de velocidad en ruedas */
        estado_carro.velocidad_rueda_izq = 80.0f + (iteracion % 3);
        estado_carro.velocidad_rueda_der = 80.0f - (iteracion % 2);
        estado_carro.datos_nuevos = 1;

        printf("[Tarea 1 - ESC (20ms)] Correccion efectuada. Rueda Izq: %.1f km/h, Rueda Der: %.1f km/h\n",
               estado_carro.velocidad_rueda_izq, estado_carro.velocidad_rueda_der);

        /* Notifica a las tareas dependientes mediante la variable condicional */
        pthread_cond_signal(&cond_datos_nuevos);

        pthread_mutex_unlock(&mutex_estado);
    }
    return NULL;
}


/* ------------------------------------------------------------------ */
/* Tarea 2: Control de Traccion (TCS) - 40 ms - POSIX Timers          */
/* ------------------------------------------------------------------ */
void* tarea_control_traccion(void* arg)
{
    struct periodic_task_timer* t = (struct periodic_task_timer*) arg;
    static int ciclo = 0;

    while (1) {
        /* Espera la llegada de la senal del temporizador */
        wait_next_activation_timer(t);

        /* Seccion Critica: Evalua traccion y ajusta posicion del acelerador */
        pthread_mutex_lock(&mutex_estado);
        t->count++;
        ciclo++;
        /* Simula ajuste de presion del pedal de aceleracion */
        estado_carro.presion_acelerador = 20.0f + (ciclo % 15);
        estado_carro.datos_nuevos = 1;

        printf("[Tarea 2 - TCS (40ms)] Control de traccion activo. Presion acelerador: %.1f%%\n",
               estado_carro.presion_acelerador);

        /* Notifica el cambio mediante la variable condicional */
        pthread_cond_signal(&cond_datos_nuevos);

        pthread_mutex_unlock(&mutex_estado);
    }
    return NULL;
}


/* ------------------------------------------------------------------ */
/* Tarea 3: Inyeccion de Combustible - 80 ms - POSIX Clocks           */
/* ------------------------------------------------------------------ */
void* tarea_inyeccion_combustible(void* arg)
{
    struct periodic_task_clock* t = (struct periodic_task_clock*) arg;

    while (1) {
        /* Espera la activacion por reloj absoluto */
        wait_next_activation_clock(t);

        pthread_mutex_lock(&mutex_estado);

        /* Si no hay datos nuevos de los sensores, espera mediante la variable condicional */
        while (!estado_carro.datos_nuevos) {
            pthread_cond_wait(&cond_datos_nuevos, &mutex_estado);
        }
        t->count++;
        /* Calcula la inyeccion en base a la presion del acelerador */
        estado_carro.inyeccion_calculada = estado_carro.presion_acelerador * 0.45f;
        estado_carro.datos_nuevos = 0; /* Consume la notificacion de datos nuevos */

        printf("[Tarea 3 - Inyeccion (80ms)] Inyeccion calculada: %.2f mg\n",
               estado_carro.inyeccion_calculada);

        pthread_mutex_unlock(&mutex_estado);
    }
    return NULL;
}


/* ------------------------------------------------------------------ */
/* Tarea 4: Monitoreo de Temperatura - 160 ms - POSIX Clocks          */
/* ------------------------------------------------------------------ */
void* tarea_monitoreo_temperatura(void* arg)
{
    struct periodic_task_clock* t = (struct periodic_task_clock*) arg;
    static int paso = 0;

    while (1) {
        /* Espera la activacion por reloj absoluto */
        wait_next_activation_clock(t);

        pthread_mutex_lock(&mutex_estado);
        t->count++;
        paso++;
        /* Simula lectura de temperatura del motor */
        estado_carro.temperatura_motor = 90.0f + (paso % 5) * 0.5f;

        printf("[Tarea 4 - Temp (160ms)] Monitoreo leve. Temp motor: %.1f C\n",
               estado_carro.temperatura_motor);

        pthread_mutex_unlock(&mutex_estado);
    }
    return NULL;
}


void imprimir_resumen(struct periodic_task_timer* t1, struct periodic_task_timer* t2,
                      struct periodic_task_clock* t3, struct periodic_task_clock* t4,
                      int segundos_ejecucion)
{
    pthread_mutex_lock(&mutex_estado);

    printf("Tarea 1 (ESC - 20ms):   Iteraciones: %d | Periodo Promedio: %.3f ms\n",
           t1->count, (float)(segundos_ejecucion * 1000) / t1->count);

    printf("Tarea 2 (TCS - 40ms):   Iteraciones: %d | Periodo Promedio: %.3f ms\n",
           t2->count, (float)(segundos_ejecucion * 1000) / t2->count);

    printf("Tarea 3 (Inj - 80ms):   Iteraciones: %d | Periodo Promedio: %.3f ms\n",
           t3->count, (float)(segundos_ejecucion * 1000) / t3->count);

    printf("Tarea 4 (Temp - 160ms): Iteraciones: %d | Periodo Promedio: %.3f ms\n",
           t4->count, (float)(segundos_ejecucion * 1000) / t4->count);

    pthread_mutex_unlock(&mutex_estado);
}

void destruir_sincronizacion(void)
{
    pthread_mutex_destroy(&mutex_estado);
    pthread_cond_destroy(&cond_datos_nuevos);
}