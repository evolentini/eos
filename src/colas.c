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

/** @file colas.c
 ** @brief Implementación de las funciones privadas para la gestion de colas de datos
 **
 **| REV | YYYY.MM.DD | Autor           | Descripción de los cambios                              |
 **|-----|------------|-----------------|---------------------------------------------------------|
 **|   2 | 2021.08.15 | evolentini      | Compatibilidad con los handlers de interrupciones       |
 **|   1 | 2021.08.14 | evolentini      | Version inicial del archivo                             |
 **
 ** @addtogroup eos
 ** @brief Sistema operativo
 ** @{ */

/* === Inclusiones de cabeceras ================================================================ */

#include "colas.h"
#include "semaforos.h"
#include <stddef.h>
#include <string.h>

/* === Definiciones y Macros =================================================================== */

/* === Declaraciones de tipos de datos internos ================================================ */

//! Estructura de datos con la información de un semaforo contador
struct eos_queue_s {
    //! Puntero al bloque de datos donde se almacenan los elementos
    void* data;
    //! Cantidad de elementos que se pueden almacenar en el bloque suministrado
    uint32_t data_count;
    //! Tamaño en bytes de cada elemento almacenado
    int32_t data_size;
    //! Indice del vector en el que se debe ingresar el siguiente dato
    int32_t index_give;
    //! Indice del vector del que se debe tomar el siguiente dato
    int32_t index_take;
    //! Semaforo para esperar cuando la cola esta llena
    eos_semaphore_t full;
    //! Semaforo para esperar cuando la cola esta vacia
    eos_semaphore_t empty;
};

/* === Declaraciones de funciones internas ===================================================== */

/**
 * @brief Busca y asigna un desciptor para una nueva tarea
 */
static eos_queue_t AllocateDescriptor(void);

/**
 * @brief Devuelve la dirección de un elemento del vector
 *
 * @param queue Puntero al descriptor de la cola de datos
 * @param index Indice del elemento que se desea acceder
 * @return Dirección de memoria del elemento solicitado
 */
static void* GetElementAddress(eos_queue_t self, uint32_t index);

/* === Definiciones de variables internas ====================================================== */

static struct eos_queue_s instances[EOS_MAX_QUEUES] = { 0 };

/* === Definiciones de variables externas ====================================================== */

/* === Definiciones de funciones internas ====================================================== */

static eos_queue_t AllocateDescriptor(void)
{
    // Variable con el resultado del descriptor de tarea asignado
    eos_queue_t self = NULL;

    for (int index = 0; index < EOS_MAX_QUEUES; index++) {
        if (instances[index].data == NULL) {
            self = &(instances[index]);
            break;
        }
    }
    return self;
}

static void* GetElementAddress(eos_queue_t self, uint32_t index)
{
    return self->data + self->data_size * index;
}

/* === Definiciones de funciones externas ====================================================== */

uint32_t QueueAvaiables(void)
{
    uint32_t result = 0;

    for (int index = 0; index < EOS_MAX_QUEUES; index++) {
        if (instances[index].data == NULL) {
            result++;
        }
    }
    return result;
}

eos_queue_t QueueCreate(void* data, uint32_t data_count, uint32_t data_size)
{
    eos_queue_t self = AllocateDescriptor();

    if (self) {
        self->data = data;
        self->data_count = data_count;
        self->data_size = data_size;
        self->full = EosSemaphoreCreate(data_count);
        self->empty = EosSemaphoreCreate(0);
    }
    return self;
}

bool QueueGive(eos_queue_t self, void const* const data)
{
    // Intenta obtener un lugar en la cola
    bool result = EosSemaphoreTake(self->full);

    // Si falla es porque la cola esta llena y estamos en una interupcion
    if (!result) {
        void* location = GetElementAddress(self, self->index_give);
        memcpy(location, data, self->data_size);
        self->index_give = (self->index_give + 1) % self->data_count;

        EosSemaphoreGive(self->empty);
    }
    return result;
}

bool QueueTake(eos_queue_t self, void* const data)
{
    // Intenta obtener un elemento de la cola
    bool result = EosSemaphoreTake(self->empty);

    // Si falla es porque la cola esta vacia y estamos en una interupcion
    if (!result) {
        void* location = GetElementAddress(self, self->index_take);
        memcpy(data, location, self->data_size);
        self->index_take = (self->index_take + 1) % self->data_count;

        EosSemaphoreGive(self->full);
    }
    return result;
}

void QueueDestroy(eos_queue_t self)
{
    // Se libera la memoria ocupada por la instancia de la cola
    memset(self, 0, sizeof(struct eos_queue_s));
}

/* === Ciere de documentacion ================================================================== */

/** @} Final de la definición del modulo para doxygen */
