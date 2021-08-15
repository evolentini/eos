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

/**
 * @brief Valor del contador para la demora en el parpadeo del led
 */
#define COUNT_DELAY 300000

/* === Declaraciones de tipos de datos internos ================================================ */

typedef struct parpadeo_s {
    gpioMap_t led;
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
void TareaA(void* data);

/**
 * @brief Función que implementa la segunda tarea del sistema
 *
 * @param[in] data  Puntero al bloque de datos para parametrizar la tarea
 */
void TareaB(void* data);

/* === Definiciones de variables internas ====================================================== */

static const struct parpadeo_s PARAMETROS[] = {
    { .led = LED1, .periodo = 2000 },
    { .led = LED2, .periodo = 1000 },
};

static eos_semaphore_t mutex;

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
 * @brief Tarea que prende un led con una tecla
 *
 * @param data Puntero a los parametros de la tarea
 */
void TareaA(void* data)
{
    (void)data;
    bool valor;

    EosTaskCreate(TareaB, (void*)&PARAMETROS[1], 1);

    while (1) {
        valor = !gpioRead(TEC4);
        gpioWrite(LED3, valor);
        EosWaitDelay(100);
    }
}

/**
 * @brief Tarea que parpadea un led con una espera pasiva
 *
 * @param data Puntero a los parametros de la tarea
 */
void TareaB(void* data)
{
    int count = 0;

    parpadeo_t parametros = data;
    while (1) {
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
    /* Creación de las tareas */
    EosTaskCreate(TareaA, NULL, 0);
    EosTaskCreate(TareaB, (void*)&PARAMETROS[0], 1);
    mutex = EosSemaphoreCreate(1);
    /* Configuración de los dispositivos de la placa */
    boardConfig();

    /* Arranque del sistemaoperativo */
    EosStartScheduler();

    return 0;
}

/* === Ciere de documentacion ================================================================== */

/** @} Final de la definición del modulo para doxygen */
