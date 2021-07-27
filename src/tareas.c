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

/** @file tareas.c
 ** @brief Implementación de las funciones para la gestion de tareas
 **
 **| REV | YYYY.MM.DD | Autor           | Descripción de los cambios                              |
 **|-----|------------|-----------------|---------------------------------------------------------|
 **|   3 | 2021.09.25 | evolentini      | Se agrega un puntero que permite parametrizar la tarea  |
 **|   2 | 2021.09.25 | evolentini      | Se mueve el cambio de contexto a la rutina PendSV       |
 **|   1 | 2021.09.25 | evolentini      | Version inicial del archivo                             |
 **
 ** @addtogroup eos
 ** @brief Sistema operativo
 ** @{ */

/* === Inclusiones de cabeceras ================================================================ */

#include "tareas.h"
#include "sapi.h"
#include <stddef.h>
#include <stdint.h>

/* === Definiciones y Macros =================================================================== */

/**
 * @brief Cantidad de máxima de tareas que se podrán crear
 */
#define TASKS_MAX_COUNT 8

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
 * @brief Función que indica un error en el cambio de contexto
 *
 * @remark Esta funcion no debería ejecutarse nunca, solo se accede a la misma si las funciones
 * que implementan las tareas terminan
 */
void TaskError(void);

/**
 * @brief Busca y asigna un desciptor para una nueva tarea
 *
 */
task_t AllocateDescriptor(void);

/* === Definiciones de variables internas ====================================================== */

/**
 * @brief Vector que almacena los descriptores de tareas
 */
static struct task_s tasks[TASKS_MAX_COUNT] = { 0 };

/**
 * @brief Vector que proporciona espacio para la pila de las tareas
 */
static uint8_t task_stacks[TASKS_MAX_COUNT][TASK_STACK_SIZE] = { 0 };

/**
 * @brief Función para preparar el contexto inicial de una tarea nueva
 *
 * @param[in]  task         Puntero al desciptor de la nueva tarea
 * @param[in]  entry_point  Punto de entrada de la función que imeplementa la tarea
 * @param[in]  data         Puntero al bloque de datos para parametrizar la tarea
 */
void PrepareContext(task_t task, task_entry_point_t entry_point, void* data);

/**
 * @brief Función para determinar la tarea a la que se otorga el procesador
 *
 * @param[in]  activa  Indice de la tarea activa hasta el momento del cambio de contexto
 * @return     int     Indice de la tarea a la que se le otorga el procesador
 */
int Schedule(int activa);

/* === Definiciones de variables externas ====================================================== */

/* === Definiciones de funciones internas ====================================================== */

void TaskError(void)
{
    gpioWrite(LEDR, true);
    while (1) { }
}

task_t AllocateDescriptor(void)
{
    // Variable con el indice de la ultima tarea creada
    static int last_created = 0;

    // Variable con el resultado del descriptor de tarea asignado
    task_t task = NULL;

    if (last_created < TASKS_MAX_COUNT) {
        task = &tasks[last_created];
        last_created++;
    }
    return task;
}

void PrepareContext(task_t task, task_entry_point_t entry_point, void* data)
{
    task->stack_pointer -= sizeof(struct task_context_s);

    task_context_t context = task->stack_pointer;
    context->context_auto.lr = (uint32_t)TaskError;
    context->context_auto.xPSR = 0x21000000;
    context->context_auto.pc = (uint32_t)entry_point;
    context->context_auto.r0 = (uint32_t)data;
}

int Schedule(int activa)
{
    do {
        activa = (activa + 1) % TASKS_MAX_COUNT;
    } while (tasks[activa].state != READY);
    return activa;
}

/* === Definiciones de funciones externas ====================================================== */

void TaskCreate(task_entry_point_t entry_point, void* data)
{
    // Variable con la ultima dirección de pila asignada
    static void* asigned_stack = task_stacks;

    // Variable con el descriptor signado a la nueva tarea
    task_t task = AllocateDescriptor();
    if (task) {
        asigned_stack += TASK_STACK_SIZE;
        task->stack_pointer = asigned_stack;
        PrepareContext(task, entry_point, data);
        task->state = READY;
    }
}

void StartScheduler(void)
{
    __asm__ volatile("cpsid i");

    /* Activate SysTick */
    SystemCoreClockUpdate();
    SysTick_Config(SystemCoreClock / 5000);

    /* Update priority set by SysTick_Config */
    NVIC_SetPriority(SysTick_IRQn, (1 << __NVIC_PRIO_BITS) - 2);
    NVIC_SetPriority(PendSV_IRQn, (1 << __NVIC_PRIO_BITS) - 1);

    __asm__ volatile("cpsie i");

    /* Espera de la primera interupción para arrancar el sistema */
    while (1) {
        __asm__ volatile("wfi");
    }
}

void SysTick_Handler(void)
{
    static int divisor = 0;
    divisor = (divisor + 1) % 1000;
    if (divisor == 0)
        gpioToggle(LEDB);

    SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;
}

__attribute__((naked())) void PendSV_Handler(void)
{
    static int activa = TASKS_MAX_COUNT;

    /* Si hay una tarea activa se salva el contexto en su correspondiente pila */
    if (tasks[activa].state == RUNNING) {
        tasks[activa].state = READY;
        __asm__ volatile("mrs r0, psp");
        __asm__ volatile("stmdb r0!, {r4-r11}");
        __asm__ volatile("str r0, %0" : "=m"(tasks[activa].stack_pointer));
    }

    /* Se determina seleciona la proxima tarea que utilizará el procesador */
    activa = Schedule(activa);

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

/* === Ciere de documentacion ================================================================== */

/** @} Final de la definición del modulo para doxygen */
