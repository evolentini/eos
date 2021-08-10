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

#ifndef EOS_API_H
#define EOS_API_H

/** @file eos_api.h
 ** @brief Declaraciones de las funciones publicas del sistema operativo
 **
 **| REV | YYYY.MM.DD | Autor           | Descripción de los cambios                              |
 **|-----|------------|-----------------|---------------------------------------------------------|
 **|   1 | 2021.08.09 | evolentini      | Version inicial del archivo                             |
 **
 ** @addtogroup modulo 
 ** @brief Breve descripción del modulo
 ** @{ */

/* === Inclusiones de archivos externos ======================================================== */

#include <stdbool.h>
#include <stdint.h>

/* === Cabecera C++ ============================================================================ */
#ifdef __cplusplus
extern "C" {
#endif

/* === Definiciones y Macros =================================================================== */

/* === Declaraciones de tipos de datos ========================================================= */

typedef enum {
    EOS_SERVICE_DELAY = 1,
} eos_services_t;

/**
 * @brief Tipo de datos con un puntero a una funcion que implementa una tarea
 */
typedef void (*eos_task_entry_point_t)(void* data);

/**
 * @brief Tipo de datos con un puntero a un descriptor de tarea
 */
typedef struct eos_task_s* eos_task_t;

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
eos_task_t EosTaskCreate(eos_task_entry_point_t entry_point, void* data, uint8_t priority);

/**
 * @brief Función para iniciar el planificador del sistema operativo
 */
void EosStartScheduler(void);

/**
 * @brief Función para esperar una cantidad de tiempo sin utilizar el procesador
 *
 * @param[in]  delay        Cantidad de tiempo en milisegundos que espera la tarea
 */
void EosWaitDelay(uint32_t delay);

/* === Ciere de documentacion ================================================================== */
#ifdef __cplusplus
}
#endif

/** @} Final de la definición del modulo para doxygen */

#endif /* EOS_API_H */
