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
 **|   3 | 2021.08.08 | evolentini      | Se agrega soporte para una tarea inactiva del sistema   |
 **|   2 | 2021.08.08 | evolentini      | Se agrega soporte para prioridades en las tareas        |
 **|   1 | 2021.08.08 | evolentini      | Version inicial del archivo                             |
 **
 ** @addtogroup eos
 ** @brief Sistema operativo
 ** @{ */

/* === Inclusiones de cabeceras ================================================================ */

#include "planificador.h"
#include <stddef.h>

/* === Definiciones y Macros =================================================================== */

/* === Declaraciones de tipos de datos internos ================================================ */

struct scheduler_s {
    task_t queue[EOS_MAX_PRIORITY][EOS_MAX_TASK_COUNT];
    task_t active_task;
    task_t background_task;
};

/* === Declaraciones de funciones internas ===================================================== */

/* === Definiciones de variables internas ====================================================== */

/* === Definiciones de variables externas ====================================================== */

static struct scheduler_s instances[1] = { 0 };

/* === Definiciones de funciones internas ====================================================== */

/* === Definiciones de funciones externas ====================================================== */

scheduler_t SchedulerCreate(task_t background_task)
{
    // Solo se puede crear una instancia del planificador
    instances->background_task = background_task;
    return instances;
}

void SchedulerEnqueue(scheduler_t self, task_t task, uint8_t priority)
{
    if (priority >= EOS_MAX_PRIORITY) {
        priority = 0;
    } else {
        priority = EOS_MAX_PRIORITY - priority - 1;
    }

    for (int index = 0; index < EOS_MAX_TASK_COUNT; index++) {
        if (self->queue[priority][index] == NULL) {
            self->queue[priority][index] = task;
            break;
        }
    }
}

task_t Schedule(scheduler_t self)
{
    uint8_t priority;

    self->active_task = self->background_task;
    for (priority = 0; priority < EOS_MAX_PRIORITY; priority++) {
        if (self->queue[priority][0] != NULL) {
            self->active_task = self->queue[priority][0];
            break;
        }
    }
    if (self->active_task != self->background_task) {
        for (int index = 0; index < EOS_MAX_TASK_COUNT - 1; index++) {
            if (self->queue[priority][index + 1] != NULL) {
                self->queue[priority][index] = self->queue[priority][index + 1];
            } else {
                self->queue[priority][index] = NULL;
                break;
            }
        }
        self->queue[priority][EOS_MAX_TASK_COUNT - 1] = NULL;
    }

    return self->active_task;
}

/* === Ciere de documentacion ================================================================== */

/** @} Final de la definición del modulo para doxygen */
