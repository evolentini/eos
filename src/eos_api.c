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

/** @file eos_api.c
 ** @brief Implementación de las funciones publicas del sistema operativo
 **
 **| REV | YYYY.MM.DD | Autor           | Descripción de los cambios                              |
 **|-----|------------|-----------------|---------------------------------------------------------|
 **|   4 | 2021.08.15 | evolentini      | Se incluyen los handlers de interrupciones              |
 **|   3 | 2021.08.14 | evolentini      | Se incluyen las funciones para manejo de colas de datos |
 **|   2 | 2021.08.09 | evolentini      | Se incluyen las funciones para manejo de semaforos      |
 **|   1 | 2021.08.08 | evolentini      | Version inicial del archivo                             |
 **
 ** @addtogroup eos
 ** @brief Sistema operativo
 ** @{ */

/* === Inclusiones de cabeceras ================================================================ */

#include "semaforos.h"
#include "tareas.h"
#include "colas.h"
#include "interrupciones.h"
#include <stddef.h>

/* === Definiciones y Macros =================================================================== */

/* === Declaraciones de tipos de datos internos ================================================ */

/* === Declaraciones de funciones internas ===================================================== */

/* === Definiciones de variables internas ====================================================== */

/* === Definiciones de variables externas ====================================================== */

/* === Definiciones de funciones internas ====================================================== */

/* === Definiciones de funciones externas ====================================================== */
eos_task_t EosTaskCreate(eos_entry_point_t entry_point, void* data, uint8_t priority)
{
    // Llama a la función privada para crear una tarea
    return TaskCreate(entry_point, data, priority);
}

void EosStartScheduler(void)
{
    // Llama a la función privada para iniciar el planificador
    StartScheduler();
}

void EosWaitDelay(uint32_t delay)
{
    if (!HandlerActive()) {
        __asm__ volatile("mov r1, %0" : : "r"(delay));
        __asm__ volatile("mov r0, %0" : : "I"(EOS_SERVICE_DELAY));
        __asm__ volatile("svc #0");
    }
}

eos_semaphore_t EosSemaphoreCreate(int32_t initial_value)
{
    // Llama a la función privada para crear un semaforo
    return SemaphoreCreate(initial_value);
}

void EosSemaphoreGive(eos_semaphore_t self)
{
    if (HandlerActive()) {
        SemaphoreGive(self);
    } else {
        __asm__ volatile("mov r1, %0" : : "r"(self));
        __asm__ volatile("mov r0, %0" : : "I"(EOS_SERVICE_GIVE));
        __asm__ volatile("svc #0");
    }
}

bool EosSemaphoreTake(eos_semaphore_t self)
{
    static bool resultado;

    if (HandlerActive()) {
        resultado = SemaphoreTake(self);
    } else {
        __asm__ volatile("mov r1, %0" : : "r"(self));
        __asm__ volatile("mov r0, %0" : : "I"(EOS_SERVICE_TAKE));
        __asm__ volatile("svc #0");
        __asm__ volatile("str r0, %0" : "=m"(resultado));
    }
    return resultado;
}

eos_queue_t EosQueueCreate(void* data, uint32_t count, uint32_t size)
{
    // Llama a la función privada
    return QueueCreate(data, count, size);
}

bool EosQueueGive(eos_queue_t queue, void* data)
{
    // Llama a la función privada
    return QueueGive(queue, data);
}

bool EosQueueTake(eos_queue_t queue, void* data)
{
    // Llama a la función privada
    return QueueTake(queue, data);
}

void EosHandlerInstall(
    uint8_t service, uint8_t prioridad, eos_entry_point_t entry_point, void* data)
{
    HandlerInstall(service, prioridad, entry_point, data);
}

void EosHandlerRemove(uint8_t service)
{
    // Llama a la función privada
    HandlerRemove(service);
}

/* === Ciere de documentacion ================================================================== */

/** @} Final de la definición del modulo para doxygen */
