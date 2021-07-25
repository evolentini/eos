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
 ** Ejemplo de la implementación básica de la ejecución de dos tareas con un planificador muy
 ** elemental tipo round robin. Se imeplementa un cambio de contexto expropiativo basado en
 ** la rutina de servicio de la interrupción del temporizador del sistema. Esta misma
 ** interrupción se utiliza para asignar las cuotas de tiempo de cada proceso.
 **
 **| REV | YYYY.MM.DD | Autor           | Descripción de los cambios                              |
 **|-----|------------|-----------------|---------------------------------------------------------|
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

#include "sapi.h"
#include "tareas.h"
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

/* === Definiciones de variables externas ====================================================== */

/* === Definiciones de funciones internas ====================================================== */

void Delay(int espera)
{
    uint32_t i;
    while (espera--) {
        for (i = COUNT_DELAY; i != 0; i--) {
            __asm__ volatile("nop");
        }
    }
}

void TareaA(void* data)
{
    (void)data;
    bool valor;
    while (1) {
        valor = !gpioRead(TEC4);
        gpioWrite(LED3, valor);
    }
}

void TareaB(void* data)
{
    parpadeo_t parametros = data;
    while (1) {
        gpioToggle(parametros->led);
        Delay(parametros->periodo);
    }
}

/* === Definiciones de funciones externas ====================================================== */

int main(void)
{
    static const struct parpadeo_s PARAMETROS[] = {
        { .led = LED1, .periodo = 10 },
        { .led = LED2, .periodo = 5 },
    };

    /* Creación de las tareas */
    TaskCreate(TareaA, NULL);
    TaskCreate(TareaB, (void*)&PARAMETROS[0]);
    TaskCreate(TareaB, (void*)&PARAMETROS[1]);

    /* Configuración de los dispositivos de la placa */
    boardConfig();

    /* Arranque del sistemaoperativo */
    StartScheduler();

    return 0;
}

/* === Ciere de documentacion ================================================================== */

/** @} Final de la definición del modulo para doxygen */
