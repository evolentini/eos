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

#ifndef COLAS_H
#define COLAS_H

/** @file colas.h
 ** @brief Declaraciones privadas del sistema operativo para la gestion de colas de datos
 **
 **| REV | YYYY.MM.DD | Autor           | Descripción de los cambios                              |
 **|-----|------------|-----------------|---------------------------------------------------------|
 **|   2 | 2021.08.15 | evolentini      | Compatibilidad con los handlers de interrupciones       |
 **|   1 | 2021.08.14 | evolentini      | Version inicial del archivo                             |
 **
 ** @addtogroup eos
 ** @brief Sistema operativo
 ** @{ */

/* === Inclusiones de archivos externos ======================================================== */

#include "eos.h"

/* === Cabecera C++ ============================================================================ */
#ifdef __cplusplus
extern "C" {
#endif

/* === Definiciones y Macros =================================================================== */

/* === Declaraciones de tipos de datos ========================================================= */

/* === Declaraciones de variables externas ===================================================== */

/* === Declaraciones de funciones externas ===================================================== */

/**
 * @brief Función interna del sistema operativo para consultar la cantidad de colas disponibles
 *
 * @return Cantidad de colas de datos disponibles para ser creadas
 */
uint32_t QueueAvaiables(void);

/**
 * @brief Función interna del sistema operativo para crear un una cola de datos
 *
 * @param[in]   data        Puntero al bloque de datos donde se almacenaran los elementos
 * @param[in]   data_count  Cantidad de elementos que se pueden almacenar en el bloque suministrado
 * @param[in]   data_size   Tamaño en bytes de cada elemento almacenado
 * @return                  Puntero al descriptor de la cola de datos creada
 */
eos_queue_t QueueCreate(void* data, uint32_t data_count, uint32_t data_size);

/**
 * @brief Función interna del sistema sistema operativo para agregar un dato en una cola
 *
 * @remark Cuando esta función se llama desde la rutina de servicio de una interrupción
 * y la cola esta llena la función retorna \p false.
 *
 * @param[in] queue     Puntero al descriptor de la cola de datos
 * @param[in] data      Puntero al bloque con el dato que se debe almacenar en la cola
 * @return \p true      El dato se pudo almacenar en la cola sin errores
 * @return \p false     El el dato no se pudo almacenar en la cola porque estaba llena
 *                      y la función se llamó desde la rutina de servicio de una interrupción
 */
bool QueueGive(eos_queue_t queue, void const* const data);

/**
 * @brief Función interna del sistema operativo para obtener un dato de una cola
 *
 * @remark Cuando esta función se llama desde la rutina de servicio de una interrupción
 * y la cola esta vacia la función retorna \p false.
 *
 * @param[in] queue     Puntero al descriptor de la cola de datos
 * @param[in] data      Puntero al bloque donde se debe almacenar el dato obtenido de la cola
 * @return \p true      El dato se pudo recuperar de la cola sin errores
 * @return \p false     El el dato no se pudo obtener de la cola porque estaba vacia
 *                      y la función se llamó desde la rutina de servicio de una interrupción
 */
bool QueueTake(eos_queue_t queue, void* const data);

/**
 * @brief Función interna del sistema operativo para destruir una cola de datos
 *
 * @param queue Puntero al descriptor de la cola de datos
 */
void QueueDestroy(eos_queue_t self);

/* === Ciere de documentacion ================================================================== */
#ifdef __cplusplus
}
#endif

/** @} Final de la definición del modulo para doxygen */

#endif /* COLAS_H */