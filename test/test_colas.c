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

/** @file test_colas.h
 ** @brief Pruebas unitarias de las funciones para la gestion de colas de datos
 **
 **| REV | YYYY.MM.DD | Autor           | Descripción de los cambios                              |
 **|-----|------------|-----------------|---------------------------------------------------------|
 **|   1 | 2021.08.14 | evolentini      | Version inicial del archivo                             |
 **
 ** @addtogroup eos
 ** @brief Sistema operativo
 ** @{ */

/* === Inclusiones de cabeceras ================================================================ */

#include "unity.h"
#include "colas.h"
#include "mock_eos_api.h"

/* === Definiciones y Macros =================================================================== */

//! Definicion con la cantidad de datos que se peuden encolar en el vector suministrado
#define DATA_COUNT 4

//! Definicion con el tamano de los datos que se encolan
#define DATA_SIZE sizeof(struct test_queue_s)

//! Macro auxiliar para desencolar datos y compararlos con los ejemplos
#define TEST_ASSERT_DEQUEUE_EQUAL_EXAMPLES(cola, desde, hasta)                                     \
    for (int index = desde; index <= hasta; index++) {                                             \
        char message[32];                                                                          \
        sprintf(message, "Index %d", index);                                                       \
        struct test_queue_s data[1];                                                               \
        QueueTake(cola, data);                                                                     \
        TEST_ASSERT_EQUAL_MEMORY_MESSAGE(&EJEMPLOS[index], data, DATA_SIZE, message);              \
    }

/* === Declaraciones de tipos de datos internos ================================================ */

//! Estructura con un ejemplo para probar colas de datos
struct test_queue_s {
    uint32_t dummy_number;
    char dummy_string[8];
};

/* === Declaraciones de funciones internas ===================================================== */

/* === Definiciones de variables internas ====================================================== */

//! Vector para el almacenamiento de los datos encolados
static struct test_queue_s data_storage[DATA_COUNT] = { 0 };

//! Variable con el descriptor de la cola usado para las pruebas
eos_queue_t cola = NULL;

//! Vector con datos de ejemplo para encolar en las pruebas
static const struct test_queue_s EJEMPLOS[] = {
    { .dummy_number = 1, .dummy_string = "UNO" },
    { .dummy_number = 2, .dummy_string = "DOS" },
    { .dummy_number = 3, .dummy_string = "TRES" },
    { .dummy_number = 4, .dummy_string = "CUATRO" },
    { .dummy_number = 5, .dummy_string = "CINCO" },
    { .dummy_number = 6, .dummy_string = "SEIS" },
};

/* === Definiciones de variables externas ====================================================== */

/* === Definiciones de funciones internas ====================================================== */

void encolar_ejemplos(eos_queue_t cola, int desde, int hasta)
{
    for (int index = desde; index <= hasta; index++) {
        QueueGive(cola, &EJEMPLOS[index]);
    }
}

/* === Definiciones de funciones externas ====================================================== */

void setUp(void)
{
    FFF_RESET_HISTORY();
    cola = QueueCreate(data_storage, DATA_COUNT, DATA_SIZE);
}

void tearDown(void)
{
    // Liberación de la cola al terminar la prueba
    QueueDestroy(cola);
}

void test_crear_una_instancia_y_destruirla(void)
{
    int avaiables = QueueAvaiables();
    eos_queue_t cola = QueueCreate(data_storage, DATA_COUNT, DATA_SIZE);
    TEST_ASSERT_EQUAL(avaiables - 1, QueueAvaiables());
    TEST_ASSERT_NOT_NULL(cola);
    QueueDestroy(cola);
    TEST_ASSERT_EQUAL(avaiables, QueueAvaiables());
}

void test_agregar_un_dato(void)
{
    // Cuando se agrega un elemento de la cola
    QueueGive(cola, &EJEMPLOS[0]);
    // Entoces se toma unidades de dos semaforos
    TEST_ASSERT_EQUAL(2, EosSemaphoreTake_fake.call_count);
    // Y se devuelven unidades en dos semaforos
    TEST_ASSERT_EQUAL(2, EosSemaphoreGive_fake.call_count);
}

void test_sacar_un_dato(void)
{
    // Cuando se retira un elemento de la cola
    struct test_queue_s recibido[1];
    QueueTake(cola, recibido);
    // Entoces se toma unidades de dos semaforos
    TEST_ASSERT_EQUAL(2, EosSemaphoreTake_fake.call_count);
    // Y se devuelven unidades en dos semaforos
    TEST_ASSERT_EQUAL(2, EosSemaphoreGive_fake.call_count);
}

void test_agregar_un_dato_y_sacarlo(void)
{
    // Cuando se agrega un elemento a la cola
    struct test_queue_s recibido[1];
    QueueGive(cola, &EJEMPLOS[0]);
    // Y se retira un elemento de la cola
    QueueTake(cola, recibido);
    // Entonces ambos elementos son iguales
    TEST_ASSERT_EQUAL_MEMORY(&EJEMPLOS[0], recibido, DATA_SIZE);
}

void test_agregar_dos_datos_y_sacarlos(void)
{
    // Cuando se agregan los dos primeros ejemplos a la cola
    encolar_ejemplos(cola, 0, 1);
    // Entonces al retirar dos elementos estos son los dos primeros ejemplos
    TEST_ASSERT_DEQUEUE_EQUAL_EXAMPLES(cola, 0, 1);
}

void test_agregar_mas_elementos_que_la_capacidad_pero_sin_llenarla(void)
{
    // Cuando se agregan los cuatro primeros ejemplos a la cola
    encolar_ejemplos(cola, 0, 3);
    // Entonces al retirar cuatro elementos estos son los cuatro primeros ejemplos
    TEST_ASSERT_DEQUEUE_EQUAL_EXAMPLES(cola, 0, 3);

    // Cuando se agrega el quinto ejemplo a la cola
    encolar_ejemplos(cola, 4, 4);
    // Entonces al retirar un elemento este es el quinto ejemplo
    TEST_ASSERT_DEQUEUE_EQUAL_EXAMPLES(cola, 4, 4);
}

/* === Ciere de documentacion ================================================================== */

/** @} Final de la definición del modulo para doxygen */
