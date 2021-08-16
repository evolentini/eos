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

#ifndef EOS_H
#define EOS_H

/** @file eos.h
 ** @brief Declaraciones publicas del sistema operativo
 **
 **| REV | YYYY.MM.DD | Autor           | Descripción de los cambios                              |
 **|-----|------------|-----------------|---------------------------------------------------------|
 **|   5 | 2021.08.14 | evolentini      | Se incluyen las definiciones para de colas de datos     |
 **|   4 | 2021.08.09 | evolentini      | Se incluyen las definiciones para semaforos             |
 **|   3 | 2021.08.09 | evolentini      | Se separan las funciones publicas y privadas del SO     |
 **|   2 | 2021.08.08 | evolentini      | Se agrega la cantidad de pioridades del sistema         |
 **|   1 | 2021.08.08 | evolentini      | Version inicial del archivo                             |
 **
 ** @addtogroup eos
 ** @brief Sistema operativo
 ** @{ */

/* === Inclusiones de archivos externos ======================================================== */

#include "eos_config.h"
#include "eos_api.h"

/* === Cabecera C++ ============================================================================ */
#ifdef __cplusplus
extern "C" {
#endif

/* === Definiciones y Macros =================================================================== */

/**
 * @brief Define la cantidad de máxima de tareas que se podrán crear
 */
#ifndef EOS_MAX_TASK_COUNT
#define EOS_MAX_TASK_COUNT 8
#elif (EOS_MAX_TASK_COUNT < 2)
#error "La cantidad minima de tareas del sistema operativo es dos"
#endif

/**
 * @brief define la cantidad de bytes asignado como pila para cada tarea
 */
#ifndef EOS_TASK_STACK_SIZE
#define EOS_TASK_STACK_SIZE 256
#elif (EOS_TASK_STACK_SIZE < 128)
#error "La cantidad mínima de byte para asignar a una tarea es de 128 bytes"
#endif

/**
 * @brief Define la máxima prioridad que se podrá asignar a una tarea
 */
#ifndef EOS_MAX_PRIORITY
#define EOS_MAX_PRIORITY 4
#elif (EOS_MAX_PRIORITY < 0 || EOS_MAX_PRIORITY > 16)
#error "La máxima prioridad de las tareas debe ser mayor o igual que 0 y menor que 16"
#endif

/**
 * @brief Define la cantidad máxima semaforos que se podran crear en el sistema
 */
#ifndef EOS_MAX_SEMAPHORES
#define EOS_MAX_SEMAPHORES 4
#elif (EOS_MAX_SEMAPHORES < 0 || EOS_MAX_SEMAPHORES > 64)
#error "La cantidad máxima de semaforos debe ser mayor o igual que 0 y menor que 64"
#endif

/**
 * @brief Define la máxima de colas de datos que se podran crear en el sistema
 */
#ifndef EOS_MAX_QUEUES
#define EOS_MAX_QUEUES 4
#elif (EOS_MAX_QUEUES < 0 || EOS_MAX_QUEUES > 64)
#error "La cantidad máxima de colas debe ser mayor o igual que 0 y menor que 64"
#elif (EOS_MAX_SEMAPHORES < 2 * EOS_MAX_QUEUES)
#error "Cada cola de datos requiere dos semaforos"
#endif

/* === Declaraciones de tipos de datos ========================================================= */

/* === Declaraciones de variables externas ===================================================== */

/* === Declaraciones de funciones externas ===================================================== */

/* === Ciere de documentacion ================================================================== */
#ifdef __cplusplus
}
#endif

/** @} Final de la definición del modulo para doxygen */

#endif /* EOS_H */
