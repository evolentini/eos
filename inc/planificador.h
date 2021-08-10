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

#ifndef PLANIFICADOR_H
#define PLANIFICADOR_H

/** @file planificador.h
 ** @brief Declaraciones privadas del sistema operativo para el planificador de tareas
 **
 **| REV | YYYY.MM.DD | Autor           | Descripción de los cambios                              |
 **|-----|------------|-----------------|---------------------------------------------------------|
 **|   3 | 2021.08.09 | evolentini      | Se separan las funciones publicas y privadas del SO     |
 **|   2 | 2021.08.08 | evolentini      | Se agrega soporte para una tarea inactiva del sistema   |
 **|   1 | 2021.08.08 | evolentini      | Version inicial del archivo                             |
 **
 ** @addtogroup eos
 ** @brief Sistema operativo
 ** @{ */

/* === Inclusiones de archivos externos ======================================================== */

#include "tareas.h"
#include <stdbool.h>
#include <stdint.h>

/* === Cabecera C++ ============================================================================ */
#ifdef __cplusplus
extern "C" {
#endif

/* === Definiciones y Macros =================================================================== */

/* === Declaraciones de tipos de datos ========================================================= */

typedef struct scheduler_s* scheduler_t;

/* === Declaraciones de variables externas ===================================================== */

/* === Declaraciones de funciones externas ===================================================== */

/**
 * @brief Funcion para crear el planificador del sistema operativo
 */
scheduler_t SchedulerCreate(eos_task_t background_task);

/**
 * @brief Función para agregar una tarea en la cola correspondiente a una prioridad
 *
 * @param   scheduler   Puntero a la instancia del planificador
 * @param   task        Puntero al descriptor de la tarea que se encola
 * @param   priority    Prioridad de la tarea que se encola
 */
void SchedulerEnqueue(scheduler_t scheduler, eos_task_t task, uint8_t priority);

/**
 * @brief Función para determinar la tarea a la que se otorga el procesador
 *
 * @param   scheduler   Puntero a la instancia del planificador
 *
 * @return  Puntero al descriptor de la tarea al que debe otorgarse el procesador
 */
eos_task_t Schedule(scheduler_t scheduler);

/* === Ciere de documentacion ================================================================== */
#ifdef __cplusplus
}
#endif

/** @} Final de la definición del modulo para doxygen */

#endif /* PLANIFICADOR_H */
