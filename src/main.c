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

/** @file main.c
 ** @brief Ejemplo de un cambio de contexto expropiativo
 **
 **| REV | YYYY.MM.DD | Autor           | Descripción de los cambios                              |
 **|-----|------------|-----------------|---------------------------------------------------------|
 **|  14 | 2021.08.14 | evolentini      | Se agrega un ejemplo de uso de las colas de datos       |
 **|  14 | 2021.08.09 | evolentini      | Se agrega un ejemplo de uso de un semaforo              |
 **|  13 | 2021.08.09 | evolentini      | Se utilizan solo las funciones publicas del SO          |
 **|  12 | 2021.08.08 | evolentini      | Se cambia para utilizar las notificaciones del sistema  |
 **|  11 | 2021.08.08 | evolentini      | Se cambia para crear tareas con diferentes prioridades  |
 **|  10 | 2021.08.08 | evolentini      | Se cambia para crear una tarea desde una tarea          |
 **|   9 | 2021.08.08 | evolentini      | Se cambia para usar espera pasiva del sistema operativo |
 **|   8 | 2021.07.25 | evolentini      | Se cambia para usar las tareas parametrizadas           |
 **|   7 | 2021.07.25 | evolentini      | Las funciones de tareas se mueven a un archivo separado |
 **|   6 | 2021.07.25 | evolentini      | Se agregan descriptores y estados para las tareas       |
 **|   5 | 2021.07.25 | evolentini      | Punteros de pila separados y modo no privilegiado       |
 **|   4 | 2021.07.09 | evolentini      | Migracion del ejemplo a firmware CIAA V2                |
 **|   3 | 2017.10.16 | evolentini      | Correción en el formato del archivo                     |
 **|   2 | 2017.09.21 | evolentini      | Cambio para utilizar drivers_bm de UNER                 |
 **|   1 | 2016.06.25 | evolentini      | Version inicial del archivo                             |
 **
 ** @addtogroup eos
 ** @brief Sistema operativo
 ** @{ */

/* === Inclusiones de cabeceras ================================================================ */

#include "eos.h"
#include "sapi.h"
#include <stdint.h>
#include <string.h>

/* === Definiciones y Macros =================================================================== */

//! Valor del contador para la demora en el parpadeo del led
#define COUNT_DELAY 300000

//! Cantidad máxima de elementos en la cola de datos
#define QUEUE_SIZE 4

//! Macro paradefinir un parametro como no utilizado
#define UNUSED(x) (void)(x)

/* === Declaraciones de tipos de datos internos ================================================ */

//! Estrcutura de datos con un led y un tiempo
typedef struct parpadeo_s {
    //! Led sobre el que se debe operar
    gpioMap_t led;
    //! Periodo de tiempo que debe permanecer encedido
    int periodo;
} * parpadeo_t;

/* === Declaraciones de funciones internas ===================================================== */

/**
 * @brief Función para generar demoras
 * Función basica que genera una demora para permitir el parpadeo de los leds
 *
 * @param[in] espera  Tiempo que demora la espera
 */
void Delay(int espera);

/**
 * @brief Función que implementa la primera tarea del sistema
 *
 * @param[in] data  Puntero al bloque de datos para parametrizar la tarea
 */
void Parpadeo(void* data);

/**
 * @brief Función que implementa la segunda tarea del sistema
 *
 * @param[in] data  Puntero al bloque de datos para parametrizar la tarea
 */
void Teclado(void* data);

/**
 * @brief Función que implementa la tercera tarea del sistema
 *
 * @param[in] data  Puntero al bloque de datos para parametrizar la tarea
 */
void Pantalla(void* data);

/* === Definiciones de variables internas ====================================================== */

//! Constante con los parametros para dos tareas diferentes a partir de la misma funcion
static const struct parpadeo_s PARAMETROS[] = {
    { .led = LED1, .periodo = 2000 },
    { .led = LED2, .periodo = 1000 },
};

//! Variable global para almacenar la referencia al descriptor al mutex de los parpadeos
static eos_semaphore_t mutex;

//! Variable global para almacenar los datos de cola
static struct parpadeo_s queue_storage[QUEUE_SIZE];

/* === Definiciones de variables externas ====================================================== */

/* === Definiciones de funciones internas ====================================================== */

/**
 * @brief Función de espera pasiva para la tarea iactiva del sistema
 *
 * @param espera Cantidad de tiempo de la espera
 */
void Delay(int espera)
{
    uint32_t i;
    while (espera--) {
        for (i = COUNT_DELAY; i != 0; i--) {
            __asm__ volatile("nop");
        }
    }
}

/**
 * @brief Función que parpadea el canal verde del led RGB cuando el sistema esta inactivo
 */
void InactiveCallback(void)
{
    gpioToggle(LEDG);
    Delay(10);
}

/**
 * @brief Función que parpadea el canal azul del led RGB cada 1000 ciclos del SysTick
 */
void SysTickCallback(void)
{
    static int divisor = 0;
    divisor = (divisor + 1) % 1000;

    if (divisor == 0) {
        gpioToggle(LEDB);
    }
}

/**
 * @brief Función que prende un led cuando una tarea termina
 *
 * @param task Puntero al descriptor de la tarea que termina
 */
void EndTaskCallback(eos_task_t task)
{
    // Enciende un led de error
    gpioToggle(LEDR);
}

/**
 * @brief Tarea revisa el teclado y encola mensajes
 *
 * @param data Puntero a los parametros de la tarea
 */
void Teclado(void* data)
{
    eos_queue_t queue = data;
    struct parpadeo_s datos[1];

    EosTaskCreate(Parpadeo, (void*)&PARAMETROS[1], 1);

    while (true) {
        datos->led = LED3;
        datos->periodo = 0;
        do {
            if (!gpioRead(TEC3)) {
                datos->periodo = 500;
            } else if (!gpioRead(TEC4)) {
                datos->periodo = 2000;
            }
            EosWaitDelay(250);
        } while (datos->periodo == 0);

        if (datos->periodo) {
            EosQueueGive(queue, datos);
        }
        do {
            EosWaitDelay(250);
        } while (!gpioRead(TEC3) || !gpioRead(TEC4));
    }
}

/**
 * @brief Tarea revisa el teclado y encola mensajes
 *
 * @param data Puntero a los parametros de la tarea
 */
void Pantalla(void* data)
{
    eos_queue_t queue = data;
    struct parpadeo_s datos[1];

    while (true) {
        EosQueueTake(queue, datos);
        gpioWrite(datos->led, true);
        EosWaitDelay(datos->periodo);

        gpioWrite(datos->led, false);
        EosWaitDelay(datos->periodo);
    }
}

/**
 * @brief Tarea que parpadea un led con una espera pasiva
 *
 * @param data Puntero a los parametros de la tarea
 */
void Parpadeo(void* data)
{
    parpadeo_t parametros = data;
    while (true) {
        EosSemaphoreTake(mutex);
        gpioWrite(parametros->led, true);
        EosWaitDelay(parametros->periodo);

        EosSemaphoreGive(mutex);
        gpioWrite(parametros->led, false);
        EosWaitDelay(parametros->periodo);
    }
}

/* === Definiciones de funciones externas ====================================================== */

int main(void)
{
    /* Variable para almacenar la referencia al descriptor de la cola de datos */
    eos_queue_t queue;

    /* Creación de las tareas */
    queue = EosQueueCreate(&queue_storage, QUEUE_SIZE, sizeof(struct parpadeo_s));
    EosTaskCreate(Teclado, queue, 0);
    EosTaskCreate(Pantalla, queue, 0);

    EosTaskCreate(Parpadeo, (void*)&PARAMETROS[0], 1);
    EosTaskCreate(Parpadeo, (void*)&PARAMETROS[1], 1);

    mutex = EosSemaphoreCreate(1);

    /* Configuración de los dispositivos de la placa */
    boardConfig();

    /* Arranque del sistemaoperativo */
    EosStartScheduler();

    return 0;
}

/* === Ciere de documentacion ================================================================== */

/** @} Final de la definición del modulo para doxygen */
