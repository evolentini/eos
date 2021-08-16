/* Copyright 2016-2021, Laboratorio de Microprocesadores
 * Facultad de Ciencias Exactas y Tecnología
 * Universidad Nacional de Tucuman
 * http://www.microprocesadores.unt.edu.ar/
 * Copyright 2016-2021, Esteban Volentini <evolentini@herrera.unt.edu.ar>
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef TAREAS_H
#define TAREAS_H

/** @file tareas.h
 ** @brief Declaraciones privadas del sistema operativo para la gestion de tareas
 **
 **| REV | YYYY.MM.DD | Autor           | Descripción de los cambios                              |
 **|-----|------------|-----------------|---------------------------------------------------------|
 **|   8 | 2021.08.09 | evolentini      | Se publican funciones necesarias implementar semaforos  |
 **|   7 | 2021.08.10 | evolentini      | Soporte para encolar las tareas con una lista enlazada  |
 **|   6 | 2021.08.09 | evolentini      | Se separan las funciones publicas y privadas del SO     |
 **|   5 | 2021.08.08 | evolentini      | Se agregan notificaciones del sistema al usuario        |
 **|   4 | 2021.08.08 | evolentini      | Se agrega soporte para prioridades en las tareas        |
 **|   3 | 2021.08.07 | evolentini      | Se agrega un servicio de espera pasiva                  |
 **|   2 | 2021.07.25 | evolentini      | Se agrega un puntero que permite parametrizar la tarea  |
 **|   1 | 2021.07.25 | evolentini      | Version inicial del archivo                             |
 **
 ** @addtogroup eos
 ** @brief Sistema operativo
 ** @{ */

/* === Inclusiones de archivos externos ======================================================== */

#include "eos.h"
#include <stdint.h>

/* === Cabecera C++ ============================================================================ */
#ifdef __cplusplus
extern "C" {
#endif

/* === Definiciones y Macros =================================================================== */

/* === Declaraciones de tipos de datos ========================================================= */

/**
 * @brief Tipo de datos enumerado con los estados de las tareas
 */
typedef enum eos_task_state_e {
    CREATING = 0,
    READY,
    WAITING,
    RUNNING,
} eos_task_state_t;

/* === Declaraciones de variables externas ===================================================== */

/* === Declaraciones de funciones externas ===================================================== */

/**
 * @brief Función para crear una nueva tarea
 *
 * @param[in]  entry_point  Puntero a la función que implementa la tarea
 * @param[in]  data         Puntero al bloque de datos para parametrizar la tarea
 * @param[in]  priority     Prioridad de la tarea que se desea crear
 *
 * @return                  Puntero al descriptor de la tarea creada
 */
eos_task_t TaskCreate(eos_entry_point_t entry_point, void* data, uint8_t priority);

/**
 * @brief Función para cambiar el estado de una tarea
 *
 * @param   task    Puntero al descriptor de la tarea que se desea cambiar de estado
 * @param   state   Nuevo estado que se asigna a la tarea
 */
void TaskSetState(eos_task_t task, eos_task_state_t state);

/**
 * @brief Función para obtener puntero al descriptor de la tarea actual
 *
 * @return Puntero al descriptor de la tarea en ejecución
 */
eos_task_t TaskGetDescriptor(void);

/**
 * @brief Función para iniciar el planificador del sistema operativo
 */
void StartScheduler(void);

/**
 * @brief Función para programar una llamada al planificado al terminar la interrupcion en curso
 */
void SchedulingRequired(void);

/**
 * @brief Función de usuario para ejecutar cuando el sistema se encuentra inactivo
 *
 * @remarks Esta función se ejecuta solo cuando el planificador no puede asignar el procesador
 * a ninguna otra tarea. Esta tarea función no debería terminar nunca y no puede utilizar ningun
 * servicio del sistema operativo ya que siempre debe permanecer en estado READY.
 */
void InactiveCallback(void);

/**
 * @brief Función de usuario para ejecutar en cada interrupcion del temporizador del sistema
 *
 * @remarks Esta función se ejecuta en modo privilegiado y en el contexto del sistema operativo
 * por lo que no se recomienda su utilización.
 */
void SysTickCallback(void);

/**
 * @brief Función de usuario para notificar la terminación de una tarea
 */
void EndTaskCallback(eos_task_t task);

/**
 * @brief Función para encolar una tarea en una cola de tareas
 *
 * @param first_task Puntero al descriptor de la primera tarea de la cola
 * @param last_task Puntero al descriptor de tarea que se agrega al final de la cola
 */
void TaskEnqueue(eos_task_t first_task, eos_task_t last_task);

/**
 * @brief Función para desencolar la primera tarea de una cola de tareas
 *
 * @param first_task Puntero al descriptor de la primera tarea de la cola
 * @return  Puntero al descriptor de la nueva primera tarea de la cola
 */
eos_task_t TaskDequeue(eos_task_t first_task);

/* === Ciere de documentacion ================================================================== */
#ifdef __cplusplus
}
#endif

/** @} Final de la definición del modulo para doxygen */

#endif /* TAREAS_H */