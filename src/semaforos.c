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

/** @file semaforos.c
 ** @brief Implementación de las funciones para la gestion de semaforos
 **
 **| REV | YYYY.MM.DD | Autor           | Descripción de los cambios                              |
 **|-----|------------|-----------------|---------------------------------------------------------|
 **|   1 | 2021.08.08 | evolentini      | Version inicial del archivo                             |
 **
 ** @addtogroup eos
 ** @brief Sistema operativo
 ** @{ */

/* === Inclusiones de cabeceras ================================================================ */

#include "semaforos.h"
#include "tareas.h"
#include <stddef.h>

/* === Definiciones y Macros =================================================================== */

//! Estructura de datos con la información de un semaforo contador
struct eos_semaphore_s {
    //! Valor actual del semaforo contador, siempre es un entero positivo
    int32_t value;
    //! Puntero a la primera tarea que espera la liberacion del semaforo
    eos_task_t waiting;
};

/* === Declaraciones de tipos de datos internos ================================================ */

/* === Declaraciones de funciones internas ===================================================== */

/**
 * @brief Busca y asigna un desciptor para una nueva tarea
 */
static eos_semaphore_t AllocateDescriptor(void);

/* === Definiciones de variables internas ====================================================== */

//! Variable local con el almacenamiento de las instancias de los semaforos
static struct eos_semaphore_s instances[EOS_MAX_SEMAPHORES] = { 0 };

/* === Definiciones de variables externas ====================================================== */

/* === Definiciones de funciones internas ====================================================== */

static eos_semaphore_t AllocateDescriptor(void)
{
    // Variable con el puntero al primer lugar vacante en las instancias
    static uint8_t first_empty = 0;

    // Variable con el resultado del descriptor de tarea asignado
    eos_semaphore_t self = NULL;

    if (first_empty < EOS_MAX_SEMAPHORES) {
        self = &(instances[first_empty]);
        first_empty++;
    }
    return self;
}

/* === Definiciones de funciones externas ====================================================== */

eos_semaphore_t SemaphoreCreate(int32_t initial_value)
{
    eos_semaphore_t self = AllocateDescriptor();

    if (self) {
        self->waiting = NULL;
        self->value = initial_value;
    }
    return self;
}

void SemaphoreGive(eos_semaphore_t self)
{
    if (self->waiting) {
        eos_task_t task = self->waiting;
        self->waiting = TaskDequeue(task);
        TaskSetState(task, READY);
        SchedulingRequired();
    } else {
        self->value++;
    }
}

void SemaphoreTake(eos_semaphore_t self)
{
    if (self->value > 0) {
        self->value--;
    } else {
        eos_task_t task = TaskGetDescriptor();
        if (self->waiting == NULL) {
            self->waiting = task;
        } else {
            TaskEnqueue(self->waiting, task);
        }
        TaskSetState(task, WAITING);
        SchedulingRequired();
    }
}

/* === Ciere de documentacion ================================================================== */

/** @} Final de la definición del modulo para doxygen */
