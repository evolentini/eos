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
#include <stdint.h>
#include <string.h>

/* === Definiciones y Macros =================================================================== */

/**
 * @brief Valor del contador para la demora en el parpadeo del led
 */
#define COUNT_DELAY 3000000

/**
 * @brief Cantidad de máxima de tareas que se podrán crear
 */
#define TASKS_MAX_COUNT 2

/**
 * @brief Cantidad de bytes asignado como pila para cada tarea
 */
#define TASK_STACK_SIZE 256

/* === Declaraciones de tipos de datos internos ================================================ */

/**
 * @brief Tipo de datos enumerado con los estados de las tareas
 */
typedef enum task_state_e {
    CREATING = 0,
    READY,
    RUNNING,
} task_state_t;

/**
 * @brief Estructura que almacena el descriptor de una tarea
 */
typedef struct task_s {
    //! Estado actual de la tarea
    task_state_t state;
    //! Copia del puntero de pila de la tarea
    void* stack_pointer;
} * task_t;

/**
 * @brief Estructura con los registros del contexto de la tarea almacenados en la pila
 */
typedef struct task_context_s {
    //! Contexto almacenado manualmente en el cambio de contexto
    struct task_context_manual_s {
        uint32_t r4;
        uint32_t r5;
        uint32_t r6;
        uint32_t r7;
        uint32_t r8;
        uint32_t r9;
        uint32_t r10;
        uint32_t r11;
    } context_manual;
    //! Contexto almacenado automaticamente por la excepción
    struct task_context_auto_s {
        uint32_t r0;
        uint32_t r1;
        uint32_t r2;
        uint32_t r3;
        uint32_t ip;
        uint32_t lr;
        uint32_t pc;
        uint32_t xPSR;
    } context_auto;
} * task_context_t;

/* === Declaraciones de funciones internas ===================================================== */

/**
 * @brief Funcion que configura el temporizador del sistema
 *
 * Esta función configura el temporizador del sistema para generar una interrupcion periodica
 * cada 2ms. Este valor es la cuota de tiempo de procesador que se asigna a cada tarea.
 */
void SysTick_Init(void);

/**
 * @brief Rutina de servicio para la interrupción del temporizador del sistema
 *
 * Esta runtina se llama por hardware cada vez que se cumple el periodo de tiempo definido en el
 * temporizador del sistema. Se aprovecha el cambio de contexto propio de la interrucpción para
 * realizar el cambio de contexto entre las tareas
 */
void SysTick_Handler(void);

/**
 * @brief Función para generar demoras
 * Función basica que genera una demora para permitir el parpadeo de los leds
 */
void Delay(void);

/**
 * @brief Función que implementa la primera tarea del sistema
 */
void TareaA(void);

/**
 * @brief Función que implementa la segunda tarea del sistema
 */
void TareaB(void);

/**
 * @brief Función para crear una nueva tarea
 *
 * @param[in]  entry_point  Puntero a la función que implementa la tarea
 */
void TaskCreate(void (*entry_point)(void));

/**
 * @brief Función que indica un error en el cambio de contexto
 *
 * @remark Esta funcion no debería ejecutarse nunca, solo se accede a la misma si las funciones
 * que implementan las tareas terminan
 */
void TaskError(void);

/* === Definiciones de variables internas ====================================================== */

/**
 * @brief Vector que almacena los descriptores de tareas
 */
static struct task_s tasks[TASKS_MAX_COUNT] = { 0 };

/**
 * @brief Vector que proporciona espacio para la pila de las tareas
 */
static uint8_t task_stacks[TASKS_MAX_COUNT][TASK_STACK_SIZE] = { 0 };

/* === Definiciones de variables externas ====================================================== */

/* === Definiciones de funciones internas ====================================================== */

void SysTick_Init(void)
{
    __asm__ volatile("cpsid i");

    /* Activate SysTick */
    SystemCoreClockUpdate();
    SysTick_Config(SystemCoreClock / 5000);

    /* Update priority set by SysTick_Config */
    NVIC_SetPriority(SysTick_IRQn, (1 << __NVIC_PRIO_BITS) - 1);

    __asm__ volatile("cpsie i");
}

__attribute__((naked())) void SysTick_Handler(void)
{
    static int divisor = 0;
    static int activa = TASKS_MAX_COUNT;

    /* Si hay una tarea activa se salva el contexto en su correspondiente pila */
    if (tasks[activa].state == RUNNING) {
        tasks[activa].state = READY;
        __asm__ volatile("mrs r0, psp");
        __asm__ volatile("stmdb r0!, {r4-r11}");
        __asm__ volatile("str r0, %0" : "=m"(tasks[activa].stack_pointer));
    }

    /* Se determina seleciona la proxima tarea que utilizará el procesador */
    activa = (activa + 1) % TASKS_MAX_COUNT;
    divisor = (divisor + 1) % 1000;
    if (divisor == 0)
        gpioToggle(LEDB);

    /* Se recupera el contexto de la tarea a ejecutar desde su correspondiente pila */
    __asm__ volatile("ldr r0, %0" : : "m"(tasks[activa].stack_pointer));
    __asm__ volatile("ldmia r0!, {r4-r11}");
    __asm__ volatile("msr psp, r0");
    __asm__ volatile("isb");

    /* Se configura el modo ejecución de la tarea como no privilegiado */
    __asm__ volatile("mrs r0, control");
    __asm__ volatile("orr r0, #1");
    __asm__ volatile("msr control, r0");
    __asm__ volatile("isb");
    tasks[activa].state = RUNNING;

    /*  Se devuelve el uso del procesador a la tarea designada */
    __asm__ volatile("ldr lr,=0xFFFFFFFD");
    __asm__ volatile("bx lr");
}

void Delay(void)
{
    uint32_t i;

    for (i = COUNT_DELAY; i != 0; i--) {
        __asm__ volatile("nop");
    }
}

void TareaA(void)
{
    bool valor;
    while (1) {
        valor = !gpioRead(TEC4);
        gpioWrite(LED3, valor);
    }
}

void TareaB(void)
{
    while (1) {
        gpioToggle(LED2);
        Delay();
    }
}

void TaskCreate(void (*entry_point)(void))
{
    // Variable con la ultima dirección de pila asignada
    static void* asigned_stack = task_stacks;
    // Variable con el indice de la ultima tarea creada
    static int last_created = 0;

    // Creación de una pila para la nueva tarea
    asigned_stack += TASK_STACK_SIZE;
    task_context_t context = asigned_stack - sizeof(struct task_context_s);
    tasks[last_created].stack_pointer = context;
    tasks[last_created].state = READY;
    last_created++;

    // Preparación del contexto inicial de la tarea
    context->context_auto.lr = (uint32_t)TaskError;
    context->context_auto.xPSR = 0x21000000;
    context->context_auto.pc = (uint32_t)entry_point;
}

void TaskError(void)
{
    gpioWrite(LEDR, true);
    while (1) { }
}

/* === Definiciones de funciones externas ====================================================== */

int main(void)
{
    /* Creación de las tareas */
    TaskCreate(TareaA);
    TaskCreate(TareaB);

    /* Configuración de los dispositivos de la placa */
    boardConfig();
    SysTick_Init();

    /* Espera de la primera interupción para arrancar el sistema */
    while (1) {
        __asm__ volatile("wfi");
    }

    return 0;
}

/* === Ciere de documentacion ================================================================== */

/** @} Final de la definición del modulo para doxygen */
