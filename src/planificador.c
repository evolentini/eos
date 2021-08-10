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

/** @file planificador.c
 ** @brief Implementación del planificador de tareas del sistema operativo
 **
 **| REV | YYYY.MM.DD | Autor           | Descripción de los cambios                              |
 **|-----|------------|-----------------|---------------------------------------------------------|
 **|   5 | 2021.08.10 | evolentini      | Uso de la lista enlazada de tareas para las colas       |
 **|   4 | 2021.08.09 | evolentini      | Se separan las funciones publicas y privadas del SO     |
 **|   3 | 2021.08.08 | evolentini      | Se agrega soporte para una tarea inactiva del sistema   |
 **|   2 | 2021.08.08 | evolentini      | Se agrega soporte para prioridades en las tareas        |
 **|   1 | 2021.08.08 | evolentini      | Version inicial del archivo                             |
 **
 ** @addtogroup eos
 ** @brief Sistema operativo
 ** @{ */

/* === Inclusiones de cabeceras ================================================================ */

#include "planificador.h"
#include "tareas.h"
#include <stddef.h>

/* === Definiciones y Macros =================================================================== */

/* === Declaraciones de tipos de datos internos ================================================ */

struct scheduler_s {
    eos_task_t queue[EOS_MAX_PRIORITY];
    eos_task_t active_task;
    eos_task_t background_task;
};

/* === Declaraciones de funciones internas ===================================================== */

/* === Definiciones de variables internas ====================================================== */

/* === Definiciones de variables externas ====================================================== */

static struct scheduler_s instances[1] = { 0 };

/* === Definiciones de funciones internas ====================================================== */

/* === Definiciones de funciones externas ====================================================== */

scheduler_t SchedulerCreate(eos_task_t background_task)
{
    // Solo se puede crear una instancia del planificador
    instances->background_task = background_task;
    return instances;
}

void SchedulerEnqueue(scheduler_t self, eos_task_t task, uint8_t priority)
{
    if (priority >= EOS_MAX_PRIORITY) {
        priority = 0;
    } else {
        priority = EOS_MAX_PRIORITY - priority - 1;
    }

    if (self->queue[priority] == NULL) {
        self->queue[priority] = task;
    } else {
        TaskEnqueue(self->queue[priority], task);
    }
}

eos_task_t Schedule(scheduler_t self)
{
    uint8_t priority;

    self->active_task = self->background_task;
    for (priority = 0; priority < EOS_MAX_PRIORITY; priority++) {
        if (self->queue[priority] != NULL) {
            self->active_task = self->queue[priority];
            self->queue[priority] = TaskDequeue(self->queue[priority]);
            break;
        }
    }

    return self->active_task;
}

/* === Ciere de documentacion ================================================================== */

/** @} Final de la definición del modulo para doxygen */
