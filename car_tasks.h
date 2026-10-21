/*************************************************/
// car_tasks.h
// Definicion de la estructura del recurso compartido
// y prototipos de las tareas del sistema.
/*************************************************/

#ifndef CAR_TASKS_H
#define CAR_TASKS_H

#include <pthread.h>
#include "rt_timing.h"

/* 
 * Recurso compartido: Estado general de los sensores y actuadores del vehiculo.
 * Este recurso es modificado por las tareas de estabilidad y traccion,
 * y procesado por las tareas de inyeccion y monitoreo.
 */
struct EstadoVehiculo {
    float velocidad_rueda_izq; /* Velocidad rueda izquierda en km/h */
    float velocidad_rueda_der; /* Velocidad rueda derecha en km/h */
    float presion_acelerador;  /* Porcentaje de presion del pedal (0 - 100%) */
    float inyeccion_calculada; /* Cantidad de combustible a inyectar en mg */
    float temperatura_motor;   /* Temperatura del motor en grados Celsius */
    int datos_nuevos;          /* Bandera para notificar actualizacion de datos */
};

/* 
 * Prototipos de las cuatro tareas requeridas por el sistema 
 */

/* Tarea 1: Control de Estabilidad (ESC/ESP) - 20 ms - POSIX Timers */
void* tarea_control_estabilidad(void* arg);

/* Tarea 2: Control de Traccion (TCS) - 40 ms - POSIX Timers */
void* tarea_control_traccion(void* arg);

/* Tarea 3: Inyeccion de Combustible - 80 ms - POSIX Clocks */
void* tarea_inyeccion_combustible(void* arg);

/* Tarea 4: Monitoreo de Temperatura (Tiempo Real Leve) - 160 ms - POSIX Clocks */
void* tarea_monitoreo_temperatura(void* arg);


void imprimir_resumen(struct periodic_task_timer* t1, struct periodic_task_timer* t2,
                      struct periodic_task_clock* t3, struct periodic_task_clock* t4,
                      int segundos_ejecucion);

void destruir_sincronizacion(void);


#endif /* CAR_TASKS_H */